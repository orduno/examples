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

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class SecondaryNode
{
public:
  explicit SecondaryNode(std::shared_ptr<rclcpp::Node> node, const std::string & name)
  : node_{node}, name_{name}
  {
    publisher_ = node_->create_publisher<std_msgs::msg::String>("secondary_node_topic", 10);
    timer_ = node_->create_wall_timer(1s, std::bind(&SecondaryNode::timer_callback, this));
  }

private:
  std::shared_ptr<rclcpp::Node> node_;
  std::string name_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;

  void timer_callback()
  {
    RCLCPP_INFO(node_->get_logger(),
      "this is %s in", name_.c_str(), node_->get_fully_qualified_name());

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
    auto sub_node_ = create_sub_node("secondary_node_ns");
    auto second_node_ = std::make_unique<SecondaryNode>(sub_node_, "secondary_node");
    // sub_node_executor_ = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();
    // sub_node_thread_ = std::make_unique<std::thread>(
    //   [&](rclcpp::Node::SharedPtr node)
    //   {
    //     sub_node_executor_->add_node(node->get_node_base_interface());
    //     sub_node_executor_->spin();
    //     sub_node_executor_->remove_node(node->get_node_base_interface());
    //   }, sub_node_);

    timer_ = create_wall_timer(1s, std::bind(&PrimaryNode::timer_callback, this));
  }

  ~PrimaryNode()
  {
    RCLCPP_INFO(get_logger(), "Destroying");
    // sub_node_executor_->cancel();
    // sub_node_thread_->join();
  }

private:
  rclcpp::Node::SharedPtr sub_node_;
  std::unique_ptr<SecondaryNode> second_node_;
  std::unique_ptr<rclcpp::executors::SingleThreadedExecutor> sub_node_executor_;
  std::unique_ptr<std::thread> sub_node_thread_;

  rclcpp::TimerBase::SharedPtr timer_;

  void timer_callback()
  {
    RCLCPP_INFO(this->get_logger(), "this is %s", this->get_fully_qualified_name());
  }

};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  // rclcpp::spin(std::make_shared<PrimaryNode>());
  auto node = std::make_shared<PrimaryNode>();
  auto executor = std::make_unique<rclcpp::executors::MultiThreadedExecutor>();
  executor->add_node(node->get_node_base_interface());
  executor->spin();
  executor->remove_node(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}
