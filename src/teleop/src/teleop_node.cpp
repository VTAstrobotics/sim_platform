#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/joy.hpp"
using std::placeholders::_1;
using namespace std::chrono_literals;

class Teleop : public rclcpp::Node
{
  public: Teleop() : Node("teleop_node")
    {
      velocity_publisher = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

      joy_subscription = this->create_subscription<sensor_msgs::msg::Joy>(
      "/joy", 10, std::bind(&Teleop::joy_callback, this, _1));
    }

  private:

    void joy_callback(sensor_msgs::msg::Joy::SharedPtr msg){

    double lin = msg->axes[axis_linear] * linear_scale;
    double ang = msg->axes[axis_angular]* angular_scale;

    geometry_msgs::msg::Twist cmd;
    cmd.linear.x = lin;
    cmd.angular.z = ang;
    velocity_publisher->publish(cmd);
    }

    double linear_scale  = 0.6;
    double angular_scale = 1.8;  
    int axis_linear   = 1;         
    int axis_angular  = 0;
    
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr velocity_publisher;
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_subscription;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Teleop>());
  rclcpp::shutdown();
  return 0;
}
