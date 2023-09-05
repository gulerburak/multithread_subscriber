#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <atomic>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

// atomic variable to increment
static std::atomic<int> shared_value = 0;

namespace MultiSub
{

  class MultiSub : public rclcpp::Node
  {
  public:
    MultiSub() : Node("multi_sub")
    {
      // create callback group to run parallel
      my_callback_group = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
      options.callback_group = my_callback_group;

      // the first subscription
      subscription_ = this->create_subscription<std_msgs::msg::String>(
          "hello",
          rclcpp::SensorDataQoS(),
          std::bind(&MultiSub::MultiSub::topic_callback, this, std::placeholders::_1),
          options);

      // the second subscription
      subscription_2 = this->create_subscription<std_msgs::msg::String>(
          "hello2",
          rclcpp::SensorDataQoS(),
          std::bind(&MultiSub::MultiSub::topic_callback2, this, std::placeholders::_1),
          options);
      // timer callbacks
      timer_ptr_ = this->create_wall_timer(500ms, std::bind(&MultiSub::MultiSub::timer_callback, this), my_callback_group);
      timer_ptr_2 = this->create_wall_timer(50ms, std::bind(&MultiSub::MultiSub::timer_callback2, this), my_callback_group);
    }

  private:
    rclcpp::TimerBase::SharedPtr timer_ptr_;
    rclcpp::TimerBase::SharedPtr timer_ptr_2;

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_2;

    rclcpp::CallbackGroup::SharedPtr my_callback_group;
    rclcpp::SubscriptionOptions options;

    void timer_callback()
    {
      shared_value += 1;
      RCLCPP_INFO(this->get_logger(), "timer callback 1");
      std::cout << "Shared value:" << shared_value << std::endl;
      std::this_thread::sleep_for(1s);
    }
    void timer_callback2()
    {
      shared_value += 1;
      RCLCPP_INFO(this->get_logger(), "timer callback 2");
      std::cout << "Shared value:" << shared_value << std::endl;
    }
    void topic_callback(const std_msgs::msg::String::SharedPtr msg)
    {
      shared_value += 1;
      RCLCPP_INFO(this->get_logger(), "callback 1");
      std::cout << "Shared value:" << shared_value << std::endl;
    }
    void topic_callback2(const std_msgs::msg::String::SharedPtr msg)
    {
      shared_value += 1;
      RCLCPP_INFO(this->get_logger(), "callback 2");
      std::cout << "Shared value:" << shared_value << std::endl;
    }
  };
}

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);

  rclcpp::executors::MultiThreadedExecutor executor;
  auto node = std::make_shared<MultiSub::MultiSub>();
  executor.add_node(node);
  RCLCPP_INFO(node->get_logger(), "Starting client node, shut down with CTRL-C");
  executor.spin();
  RCLCPP_INFO(node->get_logger(), "Keyboard interrupt, shutting down.\n");
  rclcpp::shutdown();
  return 0;
}