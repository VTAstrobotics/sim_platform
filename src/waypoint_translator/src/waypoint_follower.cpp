#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/nav_sat_fix.hpp"

#include "rclcpp_action/rclcpp_action.hpp"

#include "robot_localization/srv/from_ll.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "geometry_msgs/msg/pose_with_covariance_stamped.hpp"

using std::placeholders::_1;

class WaypointSubscriber : public rclcpp::Node
{
public:
    WaypointSubscriber()
        : Node("waypoint_subscriber")
    {
        gps_subscription = this->create_subscription<sensor_msgs::msg::NavSatFix>(
            "/gps_waypoint", 10, std::bind(&WaypointSubscriber::gps_waypoint_callback, this, _1));

        from_ll_client = this->create_client<robot_localization::srv::FromLL>("/fromLL");

        nav_client =
            rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(this, "/navigate_to_pose");

        pose_sub = this->create_subscription<geometry_msgs::msg::PoseWithCovarianceStamped>("/amcl_pose", 10,[this](geometry_msgs::msg::PoseWithCovarianceStamped::SharedPtr msg)
        {
            current_pose = msg->pose.pose;
            pose_received = true;
        });
    }

private:
    void gps_waypoint_callback(sensor_msgs::msg::NavSatFix::SharedPtr msg)
    {

        rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr pose_sub;
        geometry_msgs::msg::Pose current_pose;

        auto request = std::make_shared<robot_localization::srv::FromLL::Request>();
        request->ll_point.latitude = msg->latitude;
        request->ll_point.longitude = msg->longitude;
        request->ll_point.altitude = msg->altitude; // could disregard this?

        if (!from_ll_client->wait_for_service(std::chrono::seconds(2)))
        {
            RCLCPP_ERROR(get_logger(), "/fromLL service not available");
            return;
        }

        auto future =
            from_ll_client->async_send_request(request, std::bind(&WaypointSubscriber::from_ll_response_callback, this, _1));
    }

    void from_ll_response_callback(rclcpp::Client<robot_localization::srv::FromLL>::SharedFuture future)
    {
        auto response = future.get();
        const auto &p = response->map_point;
        RCLCPP_INFO(this->get_logger(), "GPS to map: x=%.3f y=%.3f z=%.3f", p.x, p.y, p.z);

        // TODO Send cartesian coordinates to nav2 action

        // Out of bounds checking starts here
        if (!pose_received)
        {
            RCLCPP_WARN(this->get_logger(), "Sim does not have /odom yet, or can't find it");
            return;
        }

        double dx = p.x - current_pose.position.x;
        double dy = p.y - current_pose.position.y;
        double dist = std::sqrt((dx * dx) + (dy * dy));

        double max_distance = 1; // This should be the max costmap radius, maybe slightly smaller? (small size rn to test it)

        double target_x = p.x;
        double target_y = p.y;

        if (dist > max_distance)
        {
            double scale = max_distance / dist;
            target_x = current_pose.position.x + dx * scale;
            target_y = current_pose.position.y + dy * scale;

            RCLCPP_WARN(this->get_logger(), "Pose too far, moving it to %.2f m.", max_distance);
        }

        nav2_msgs::action::NavigateToPose::Goal goal;
        goal.pose.header.frame_id = "map";
        goal.pose.header.stamp = this->now();
        goal.pose.pose.position.x = target_x;
        goal.pose.pose.position.y = target_y;
        goal.pose.pose.position.z = 0.0;
        goal.pose.pose.orientation.w = 1.0;

        rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SendGoalOptions options;

        options.goal_response_callback =
            [this](rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr goal_handle)
        {
            if (!goal_handle)
            {
                RCLCPP_ERROR(this->get_logger(), "Goal rejected by Nav2");
            }
            else
            {
                RCLCPP_INFO(this->get_logger(), "Goal accepted by Nav2");
            }
        };

        // options.feedback_callback =
        //     [this](rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr,
        //            const std::shared_ptr<const nav2_msgs::action::nav2_msgs::action::NavigateToPose::Feedback> feedback)
        // {

        // };

        options.result_callback =
            [this](const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult &result)
        {
            switch (result.code)
            {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "Nav2 reached goal");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Nav2 aborted");
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_WARN(this->get_logger(), "Nav2 canceled");
                break;
            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown result code");
                break;
            }
        };

        if (!nav_client->wait_for_action_server(std::chrono::seconds(2)))
        {
            RCLCPP_ERROR(this->get_logger(), "Nav2 action server not available");
            return;
        }

        nav_client->async_send_goal(goal, options);
    }
    rclcpp::Subscription<sensor_msgs::msg::NavSatFix>::SharedPtr gps_subscription;
    rclcpp::Client<robot_localization::srv::FromLL>::SharedPtr from_ll_client;
    rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr nav_client;
    rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr pose_sub;
    geometry_msgs::msg::Pose current_pose;
    bool pose_received = false;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<WaypointSubscriber>());
    rclcpp::shutdown();
    return 0;
}

/**
 *
 * ros2 topic pub --once /gps_waypoint sensor_msgs/msg/NavSatFix "
header:
  frame_id: 'gps'
status:
  status: 0
  service: 1
latitude: -37.4275
longitude: 122.1697
altitude: 0.0
position_covariance: [0.0, 0.0, 0.0,
                      0.0, 0.0, 0.0,
                      0.0, 0.0, 0.0]
position_covariance_type: 0
"

 */
