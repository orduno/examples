"""Launch a talker and a listener."""

from launch import LaunchDescription
from launch_ros.actions import PushRosNamespace
from launch.actions import GroupAction

import launch.actions
import launch.substitutions
import launch_ros.actions


def generate_launch_description():
    """Launch a talker and a listener."""
    return LaunchDescription([
        GroupAction([
            PushRosNamespace('launch_ns'),
            launch_ros.actions.Node(
                package='examples_rclcpp_minimal_timer',
                node_executable='timer_member_function',
                output='screen')
        ])
    ])

# def generate_launch_description():
#     """Launch a talker and a listener."""
#     return LaunchDescription([
#         launch_ros.actions.Node(
#             package='examples_rclcpp_minimal_timer',
#             node_executable='timer_member_function',
#             output='screen')
#     ])
