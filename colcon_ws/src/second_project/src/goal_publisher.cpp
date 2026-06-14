#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "ament_index_cpp/get_package_share_directory.hpp"

class GoalPublisher : public rclcpp::Node {
public:
    using NavigateToPose = nav2_msgs::action::NavigateToPose;
    using GoalHandleNav = rclcpp_action::ClientGoalHandle<NavigateToPose>;

    GoalPublisher() : Node("goal_publisher"), current_goal_index_(0) {
        client_ptr_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");

        // Load the CSV file
        std::string pkg_share = ament_index_cpp::get_package_share_directory("second_project");
        std::string csv_path = pkg_share + "/csv/goals.csv";
        load_csv(csv_path);

        // Wait for the action server to be up
        RCLCPP_INFO(this->get_logger(), "Waiting for 'navigate_to_pose' action server...");
        client_ptr_->wait_for_action_server();
        RCLCPP_INFO(this->get_logger(), "Action server is ready.");

        // Start the sequence
        send_next_goal();
    }

private:
    struct GoalPose {
        double x, y, theta;
    };

    rclcpp_action::Client<NavigateToPose>::SharedPtr client_ptr_;
    std::vector<GoalPose> goals_;
    size_t current_goal_index_;

    void load_csv(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            RCLCPP_ERROR(this->get_logger(), "Failed to open CSV file: %s", path.c_str());
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string val;
            GoalPose g;
            
            // Expected structure: x, y, theta
            if (std::getline(ss, val, ',')) g.x = std::stod(val);
            if (std::getline(ss, val, ',')) g.y = std::stod(val);
            if (std::getline(ss, val, ',')) g.theta = std::stod(val);
            
            goals_.push_back(g);
        }
        RCLCPP_INFO(this->get_logger(), "Loaded %zu goals from CSV.", goals_.size());
    }

    void send_next_goal() {
        if (current_goal_index_ >= goals_.size()) {
            RCLCPP_INFO(this->get_logger(), "All goals have been processed!");
            rclcpp::shutdown();
            return;
        }

        auto current_goal = goals_[current_goal_index_];
        auto goal_msg = NavigateToPose::Goal();

        goal_msg.pose.header.frame_id = "map";
        goal_msg.pose.header.stamp = this->now();
        goal_msg.pose.pose.position.x = current_goal.x;
        goal_msg.pose.pose.position.y = current_goal.y;
        goal_msg.pose.pose.position.z = 0.0;

        // Convert theta (yaw) to quaternion
        tf2::Quaternion q;
        q.setRPY(0, 0, current_goal.theta);
        goal_msg.pose.pose.orientation.x = q.x();
        goal_msg.pose.pose.orientation.y = q.y();
        goal_msg.pose.pose.orientation.z = q.z();
        goal_msg.pose.pose.orientation.w = q.w();

        RCLCPP_INFO(this->get_logger(), "Sending Goal %zu: [x: %.2f, y: %.2f, theta: %.2f]", 
                    current_goal_index_ + 1, current_goal.x, current_goal.y, current_goal.theta);

        auto send_goal_options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
        send_goal_options.result_callback = std::bind(&GoalPublisher::result_callback, this, std::placeholders::_1);

        client_ptr_->async_send_goal(goal_msg, send_goal_options);
    }

    void result_callback(const GoalHandleNav::WrappedResult & result) {
        switch (result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "Goal reached!");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Goal was aborted.");
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_ERROR(this->get_logger(), "Goal was canceled.");
                break;
            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown result code.");
                break;
        }
        
        // Progress to next goal regardless of success/abort
        current_goal_index_++;
        send_next_goal();
    }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<GoalPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}