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
#include "rclcpp_lifecycle/lifecycle_node.hpp"

using namespace std::chrono_literals;

class MinimalLifecycleTimer : public rclcpp_lifecycle::LifecycleNode
{
public:
  MinimalLifecycleTimer()
  : rclcpp_lifecycle::LifecycleNode("minimal_life_cycle_timer", "lifecycle_timer_ns", rclcpp::NodeOptions())
  {
    RCLCPP_INFO(this->get_logger(), "The namespace is %s", this->get_namespace());

    timer_ = create_wall_timer(
      500ms, std::bind(&MinimalLifecycleTimer::timer_callback, this));
  }

private:
  void timer_callback()
  {
    RCLCPP_INFO(this->get_logger(), "Hello, world!");
  }
  rclcpp::TimerBase::SharedPtr timer_;
};

class MinimalTimer : public rclcpp::Node
{
public:
  MinimalTimer()
  : Node("minimal_timer", "timer_ns")
  {
    RCLCPP_INFO(this->get_logger(), "The namespace is %s", this->get_namespace());

    timer_ = create_wall_timer(
      500ms, std::bind(&MinimalTimer::timer_callback, this));
  }

private:
  void timer_callback()
  {
    RCLCPP_INFO(this->get_logger(), "Hello, world!");
  }
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto minimal_timer = std::make_shared<MinimalTimer>();
  auto minimal_lifecycle_timer = std::make_shared<MinimalLifecycleTimer>();

  auto exec = std::make_unique<rclcpp::executors::MultiThreadedExecutor>();
  exec->add_node(minimal_timer->get_node_base_interface());
  exec->add_node(minimal_lifecycle_timer->get_node_base_interface());
  exec->spin();

  rclcpp::shutdown();
  return 0;
}
