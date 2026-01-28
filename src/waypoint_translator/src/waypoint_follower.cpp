#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/nav_sat_fix.hpp"

#include "robot_localization/srv/from_ll.hpp"

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
    rclcpp::Subscription<sensor_msgs::msg::NavSatFix>::SharedPtr gps_subscription;
    rclcpp::Client<robot_localization::srv::FromLL>::SharedPtr from_ll_client;

    void from_ll_response_callback(rclcpp::Client<robot_localization::srv::FromLL>::SharedFuture future)
    {
        auto response = future.get();
        const auto &p = response->map_point;
        RCLCPP_INFO(this->get_logger(), "GPS to map: x=%.3f y=%.3f z=%.3f", p.x, p.y, p.z);

        //TODO Send cartesian coordinates to nav2 action
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<WaypointSubscriber>());
    rclcpp::shutdown();
    return 0;
}