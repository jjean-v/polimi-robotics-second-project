#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <fstream>
#include <sstream>
#include <vector>

struct Goal { double x, y, theta; };

class GoalPublisher : public rclcpp::Node {
public:
    using NavigateToPose = nav2_msgs::action::NavigateToPose;
    using GoalHandleNavigateToPose = rclcpp_action::ClientGoalHandle<NavigateToPose>;

    // FIX 1: Allow the launch file to pass 'use_sim_time' without crashing the node
    GoalPublisher() : Node("goal_publisher", rclcpp::NodeOptions().allow_undeclared_parameters(true)) {
        action_client_ = rclcpp_action::create_client<NavigateToPose>(this, "navigate_to_pose");
        
        std::string pkg_share = ament_index_cpp::get_package_share_directory("second_project");
        load_goals(pkg_share + "/csv/goals.csv");
        
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&GoalPublisher::check_server_and_send, this));
    }

private:
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::TimerBase::SharedPtr delay_timer_;
    rclcpp_action::Client<NavigateToPose>::SharedPtr action_client_;
    std::vector<Goal> goals_;
    size_t current_goal_index_ = 0;

    void load_goals(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            RCLCPP_ERROR(this->get_logger(), "Failed to open CSV: %s", filepath.c_str());
            return;
        }

        std::string line;
        if (std::getline(file, line)) {
            RCLCPP_INFO(this->get_logger(), "Skipped CSV header: %s", line.c_str());
        }

        while (std::getline(file, line)) {
            if (line.empty()) continue; 

            std::stringstream ss(line);
            std::string x_str, y_str, theta_str;
            if (std::getline(ss, x_str, ',') && std::getline(ss, y_str, ',') && std::getline(ss, theta_str, ',')) {
                try {
                    goals_.push_back({std::stod(x_str), std::stod(y_str), std::stod(theta_str)});
                } catch (const std::exception& e) {
                    RCLCPP_WARN(this->get_logger(), "Could not parse line: %s", line.c_str());
                }
            }
        }
        RCLCPP_INFO(this->get_logger(), "Loaded %zu goals", goals_.size());
    }

    void check_server_and_send() {
        if (!action_client_->action_server_is_ready()) {
            RCLCPP_INFO(this->get_logger(), "Waiting for Nav2 action server 'navigate_to_pose' to be ready...");
            return;
        }

        // The server is on the network, but Nav2 might still be transitioning to ACTIVE.
        RCLCPP_INFO(this->get_logger(), "Action server found! Waiting 5 seconds for Nav2 lifecycle to fully activate...");
        timer_->cancel();
        
        // Wait 5 seconds using a one-shot timer before sending the first goal
        delay_timer_ = this->create_wall_timer(
            std::chrono::seconds(5),
            [this]() {
                delay_timer_->cancel();
                send_next_goal();
            });
    }

    void send_next_goal() {
        if (current_goal_index_ >= goals_.size()) {
            RCLCPP_INFO(this->get_logger(), "All goals processed! Shutting down node.");
            rclcpp::shutdown();
            return;
        }

        auto goal_msg = NavigateToPose::Goal();
        goal_msg.pose.header.frame_id = "map";
        goal_msg.pose.header.stamp = this->now();
        goal_msg.pose.pose.position.x = goals_[current_goal_index_].x;
        goal_msg.pose.pose.position.y = goals_[current_goal_index_].y;

        tf2::Quaternion q;
        q.setRPY(0, 0, goals_[current_goal_index_].theta);
        goal_msg.pose.pose.orientation.x = q.x();
        goal_msg.pose.pose.orientation.y = q.y();
        goal_msg.pose.pose.orientation.z = q.z();
        goal_msg.pose.pose.orientation.w = q.w();

        auto send_goal_options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
        send_goal_options.result_callback = std::bind(&GoalPublisher::result_callback, this, std::placeholders::_1);

        RCLCPP_INFO(this->get_logger(), "Sending goal %zu: [x: %.2f, y: %.2f, theta: %.2f]", 
            current_goal_index_ + 1, goals_[current_goal_index_].x, goals_[current_goal_index_].y, goals_[current_goal_index_].theta);
        action_client_->async_send_goal(goal_msg, send_goal_options);
    }

    void result_callback(const GoalHandleNavigateToPose::WrappedResult & result) {
        switch (result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "Goal reached successfully!");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Goal was aborted.");
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_ERROR(this->get_logger(), "Goal was canceled.");
                break;
            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown result code");
                break;
        }
        current_goal_index_++;
        send_next_goal(); 
    }
};

int main(int argc, char ** argv) {
    // FIX 2: Force stdout to flush immediately so logs are never swallowed
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<GoalPublisher>());
    rclcpp::shutdown();
    return 0;
}