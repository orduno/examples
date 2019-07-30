// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class Helper
{
public:
  explicit Helper(rclcpp::Node::SharedPtr node)
  : node_{node}
  {
    publisher_ = node_->create_publisher<std_msgs::msg::String>("helper_topic", 10);

    timer_ = node_->create_wall_timer(1s, std::bind(&Helper::timer_callback, this));
  }

private:
  std::shared_ptr<rclcpp::Node> node_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;

  void timer_callback()
  {
    RCLCPP_INFO(node_->get_logger(),
      "helper has namespace %s", node_->get_effective_namespace().c_str());

    auto message = std_msgs::msg::String();
    message.data = "this is helper";
    publisher_->publish(message);
  }
};

class MinimalTimer : public rclcpp::Node
{
public:
  MinimalTimer()
  : Node("minimal_timer", "timer_ns")
  {
    RCLCPP_INFO(this->get_logger(), "The namespace is %s", this->get_namespace());

    sub_node_ = create_sub_node("helper_ns");

    helper_ = std::make_unique<Helper>(sub_node_);

    timer_ = create_wall_timer(
      500ms, std::bind(&MinimalTimer::timer_callback, this));
  }

private:
  rclcpp::Node::SharedPtr sub_node_;
  std::unique_ptr<Helper> helper_;

  rclcpp::TimerBase::SharedPtr timer_;

  void timer_callback()
  {
    RCLCPP_INFO(this->get_logger(), "minimal timer has namespace %s",
    get_effective_namespace().c_str());
  }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalTimer>());
  rclcpp::shutdown();
  return 0;
}
