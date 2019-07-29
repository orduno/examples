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
#include <string>
#include <iostream>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class Util
{
public:
  explicit Util(rclcpp::Node::SharedPtr node, const std::string & name)
  : node_{node}, name_{name}
  {
    publisher_ = node_->create_publisher<std_msgs::msg::String>("util_topic", 10);

    timer_ = node_->create_wall_timer(1s, std::bind(&Util::timer_callback, this));
  }

private:
  std::shared_ptr<rclcpp::Node> node_;
  std::string name_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;

  void timer_callback()
  {
    RCLCPP_INFO(node_->get_logger(),
      "this is %s in %s", node_->get_effective_namespace().c_str(), name_.c_str());

    auto message = std_msgs::msg::String();
    message.data = "this is " + name_;
    publisher_->publish(message);
  }
};

class PrimaryNode : public rclcpp::Node
{
public:
  PrimaryNode()
  : Node("primary_node")
  {
    sub_node_ = create_sub_node("util_ns");

    util_ = std::make_unique<Util>(sub_node_, "sub_node");

    timer_ = create_wall_timer(1s, std::bind(&PrimaryNode::timer_callback, this));

    sub_node_timer_ = sub_node_->create_wall_timer(
      1s, std::bind(&PrimaryNode::sub_node_timer_callback, this));
  }

  ~PrimaryNode()
  {
    RCLCPP_INFO(get_logger(), "PrimaryNode::Destroying");
  }

private:
  rclcpp::Node::SharedPtr sub_node_;
  std::unique_ptr<Util> util_;

  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::TimerBase::SharedPtr sub_node_timer_;

  void timer_callback()
  {
    RCLCPP_INFO(this->get_logger(), "this is %s", get_effective_namespace().c_str());
  }

    void sub_node_timer_callback()
  {
    RCLCPP_INFO(this->get_logger(), "this is %s", sub_node_->get_effective_namespace().c_str());
  }

};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<PrimaryNode>();

  auto executor = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
  executor->add_node(node->get_node_base_interface());
  executor->spin();
  executor->remove_node(node->get_node_base_interface());

  rclcpp::shutdown();
  return 0;
}
