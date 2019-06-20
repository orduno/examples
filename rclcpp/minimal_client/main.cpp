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
#include <cinttypes>
#include <memory>
#include <future>
#include "example_interfaces/srv/add_two_ints.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class MinimalClient : public rclcpp::Node
{
public:
  using AddTwoInts = example_interfaces::srv::AddTwoInts;
  using ServiceResponseFuture = rclcpp::Client<AddTwoInts>::SharedFuture;

  explicit MinimalClient(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("MinimalClient", options)
  {
    client_ = create_client<AddTwoInts>("add_two_ints");
    timer_ = create_wall_timer(10s, std::bind(&MinimalClient::on_timer, this));
  }

private:
  rclcpp::Client<AddTwoInts>::SharedPtr client_;
  rclcpp::TimerBase::SharedPtr timer_;

  void on_timer()
  {
    if (!client_->wait_for_service(1s)) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(
          this->get_logger(),
          "Interrupted while waiting for the service. Exiting.");
        return;
      }
      RCLCPP_INFO(this->get_logger(), "Service not available after waiting");
      return;
    }

    // Choose one of the following ways to send the service and obtain the response
    // doFirstAlternative();
    // doSecondAlternative();
    doThirdAlternative();

    RCLCPP_INFO(this->get_logger(), "Exiting timer callback");
  }

  void doFirstAlternative()
  {
    // Option 1: Pass a callback to async
    auto request = std::make_shared<AddTwoInts::Request>();
    request->a = 2;
    request->b = 3;

    auto response_received_callback = [this](ServiceResponseFuture future) {
        RCLCPP_INFO(this->get_logger(), "Got result: [%" PRId64 "]", future.get()->sum);
    };

    auto future = client_->async_send_request(request, response_received_callback);
  }

  void doSecondAlternative()
  {
    // Option 2: Start a separate thread to wait for the future
    auto request = std::make_shared<AddTwoInts::Request>();
    request->a = 2;
    request->b = 3;

    auto future = client_->async_send_request(request);
    RCLCPP_INFO(this->get_logger(), "Sent request");

    std::thread wait_for_future_thread([this, future]() {
      RCLCPP_INFO(this->get_logger(), "Waiting...");
      std::future_status status;
      do {
          status = future.wait_for(1s);
          if (status == std::future_status::deferred) {
              RCLCPP_INFO(this->get_logger(), "Deferred");
          } else if (status == std::future_status::timeout) {
              RCLCPP_INFO(this->get_logger(), "Timeout");
          } else if (status == std::future_status::ready) {
              RCLCPP_INFO(this->get_logger(), "Ready");
          }
      } while (status != std::future_status::ready && rclcpp::ok());
      RCLCPP_INFO(this->get_logger(), "Exiting while loop");

      if (status == std::future_status::ready) {
        RCLCPP_INFO(this->get_logger(), "Got result: [%" PRId64 "]", future.get()->sum);
      }

    });
    wait_for_future_thread.detach();
  }

  void doThirdAlternative()
  {
    // Option 3: Run the node on a multithreaded executor
    //           and assume another thread will get process the response and update the future
    auto request = std::make_shared<AddTwoInts::Request>();
    request->a = 2;
    request->b = 3;

    auto future = client_->async_send_request(request);
    RCLCPP_INFO(this->get_logger(), "Sent request");

    RCLCPP_INFO(this->get_logger(), "Waiting...");
    std::future_status status;
    do {
        // Wait for the other thread on the executor to catch the client executable
        // and update the future
        status = future.wait_for(1s);
        if (status == std::future_status::deferred) {
            RCLCPP_INFO(this->get_logger(), "Deferred");
        } else if (status == std::future_status::timeout) {
            RCLCPP_INFO(this->get_logger(), "Timeout");
        } else if (status == std::future_status::ready) {
            RCLCPP_INFO(this->get_logger(), "Ready");
        }
    } while (status != std::future_status::ready && rclcpp::ok());
    RCLCPP_INFO(this->get_logger(), "Exiting while loop");

    if (status == std::future_status::ready) {
      RCLCPP_INFO(this->get_logger(), "Got result: [%" PRId64 "]", future.get()->sum);
    }
  }

};  // class MinimalClient

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto client = std::make_shared<MinimalClient>();

  // rclcpp::executors::MultiThreadedExecutor exec(rclcpp::executor::ExecutorArgs(), 2);
  rclcpp::executors::SingleThreadedExecutor exec;
  exec.add_node(client->get_node_base_interface());
  exec.spin();

  rclcpp::shutdown();
  return 0;
}
