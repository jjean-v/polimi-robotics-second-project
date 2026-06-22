#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <nav2_msgs/action/navigate_to_pose.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <ament_index_cpp/get_package_share_directory.hpp>

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>

using namespace std::chrono_literals;

struct Goal {
    double x;
    double y;
    double theta;
};

class GoalPublisher : public rclcpp::Node {
public:
    using NavigateToPose = nav2_msgs::action::NavigateToPose;
    using GoalHandleNavigateToPose = rclcpp_action::ClientGoalHandle<NavigateToPose>;

    GoalPublisher() : Node("goal_publisher"), current_goal_index_(0) {
        // Initialize the Action Client
        action_client_ = rclcpp_action::create_client<NavigateToPose>(
            this,
            "navigate_to_pose");

        // Load the goals from CSV
        if (!load_csv()) {
            RCLCPP_ERROR(this->get_logger(), "Failed to load CSV file. Shutting down.");
            rclcpp::shutdown();
            return;
        }

        // Create a 5-second polling timer before checking if the action server is up.
        timer_ = this->create_wall_timer(
            5s, std::bind(&GoalPublisher::check_server_and_start, this));
    }

private:
    rclcpp_action::Client<NavigateToPose>::SharedPtr action_client_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::vector<Goal> goals_;
    size_t current_goal_index_;

    bool load_csv() {
        std::string pkg_share;
        try {
            pkg_share = ament_index_cpp::get_package_share_directory("second_project");
        } catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Package 'second_project' not found.");
            return false;
        }

        std::string csv_path = pkg_share + "/csv/goals.csv";
        std::ifstream file(csv_path);

        if (!file.is_open()) {
            RCLCPP_ERROR(this->get_logger(), "Could not open file: %s", csv_path.c_str());
            return false;
        }

        std::string line;
        
        // Read the first line immediately to consume the "x,y,theta" header
        if (std::getline(file, line)) {
            RCLCPP_INFO(this->get_logger(), "Skipped CSV header: %s", line.c_str());
        }
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string val;
            Goal g;

            try {
                std::getline(ss, val, ','); g.x = std::stod(val);
                std::getline(ss, val, ','); g.y = std::stod(val);
                std::getline(ss, val, ','); g.theta = std::stod(val);
                goals_.push_back(g);
            } catch (const std::exception& e) {
                RCLCPP_WARN(this->get_logger(), "Skipping invalid row: %s", line.c_str());
            }
        }

        RCLCPP_INFO(this->get_logger(), "Successfully loaded %zu goals.", goals_.size());
        return !goals_.empty();
    }

    void check_server_and_start() {
        
        // Checks if the server is available
        if (!action_client_->action_server_is_ready()) {
            RCLCPP_INFO(this->get_logger(), "Waiting for Nav2 action server to come online...");
            return; // Exit callback, it loops
        }

        // server is ready
        RCLCPP_INFO(this->get_logger(), "Nav2 Action Server is ONLINE! Starting navigation sequence.");
        timer_->cancel();
        
        // Send the first goal
        send_next_goal();
    }

    void send_next_goal() {
        if (current_goal_index_ >= goals_.size()) {
            RCLCPP_INFO(this->get_logger(), "All goals have been processed! Task Complete.");
            rclcpp::shutdown();
            return;
        }

        auto current_goal = goals_[current_goal_index_];
        RCLCPP_INFO(this->get_logger(), "Sending Goal %zu: [x: %.2f, y: %.2f, theta: %.2f]", 
            current_goal_index_ + 1, current_goal.x, current_goal.y, current_goal.theta);

        auto goal_msg = NavigateToPose::Goal();
        goal_msg.pose.header.frame_id = "map";
        goal_msg.pose.header.stamp = this->now();

        goal_msg.pose.pose.position.x = current_goal.x;
        goal_msg.pose.pose.position.y = current_goal.y;
        goal_msg.pose.pose.position.z = 0.0;

        tf2::Quaternion q;
        q.setRPY(0, 0, current_goal.theta);
        goal_msg.pose.pose.orientation.x = q.x();
        goal_msg.pose.pose.orientation.y = q.y();
        goal_msg.pose.pose.orientation.z = q.z();
        goal_msg.pose.pose.orientation.w = q.w();

        auto send_goal_options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();
        
        send_goal_options.goal_response_callback = 
            std::bind(&GoalPublisher::goal_response_callback, this, std::placeholders::_1);
            
        send_goal_options.result_callback =
            std::bind(&GoalPublisher::result_callback, this, std::placeholders::_1);

        action_client_->async_send_goal(goal_msg, send_goal_options);
    }

    void goal_response_callback(const GoalHandleNavigateToPose::SharedPtr & goal_handle) {
        if (!goal_handle) {
            RCLCPP_ERROR(this->get_logger(), "Goal was rejected by the server.");
            // Advance to the next goal instead of getting permanently stuck
            current_goal_index_++;
            send_next_goal();
        } else {
            RCLCPP_INFO(this->get_logger(), "Goal accepted by server, moving...");
        }
    }

    void result_callback(const GoalHandleNavigateToPose::WrappedResult & result) {
        switch (result.code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "Goal was Reached!");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Goal was Aborted!");
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_ERROR(this->get_logger(), "Goal was Canceled!");
                break;
            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown result code!");
                break;
        }

        current_goal_index_++;
        send_next_goal();
    }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<GoalPublisher>());
    rclcpp::shutdown();
    return 0;
}