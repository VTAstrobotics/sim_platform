#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/nav_sat_fix.hpp"

#include "rclcpp_action/rclcpp_action.hpp"

#include "robot_localization/srv/from_ll.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"

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
            rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(this, "navigate_to_pose");
    }

private:
    void gps_waypoint_callback(sensor_msgs::msg::NavSatFix::SharedPtr msg)
    {

        auto request = std::make_shared<robot_localization::srv::FromLL::Request>();
        request->ll_point.latitude = msg->latitude;
        request->ll_point.longitude = msg->longitude;
        request->ll_point.altitude = msg->altitude; // could disregard this?

        auto future =
            from_ll_client->async_send_request(request, std::bind(&WaypointSubscriber::from_ll_response_callback, this, _1));
    }

    void from_ll_response_callback(rclcpp::Client<robot_localization::srv::FromLL>::SharedFuture future)
    {
        auto response = future.get();
        const auto &p = response->map_point;
        RCLCPP_INFO(this->get_logger(), "GPS to map: x=%.3f y=%.3f z=%.3f", p.x, p.y, p.z);

        // TODO Send cartesian coordinates to nav2 action

        nav2_msgs::action::NavigateToPose::Goal goal;
        goal.pose.header.frame_id = "map";
        goal.pose.header.stamp = this->now();
        goal.pose.pose.position.x = p.x;
        goal.pose.pose.position.y = p.y;
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
        nav_client->async_send_goal(goal, options);
    }

    rclcpp::Subscription<sensor_msgs::msg::NavSatFix>::SharedPtr gps_subscription;
    rclcpp::Client<robot_localization::srv::FromLL>::SharedPtr from_ll_client;
    rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr nav_client;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<WaypointSubscriber>());
    rclcpp::shutdown();
    return 0;
}