#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm>

using namespace std;

// Constants for strings
const string ENTER_USERNAME = "Enter Username: ";
const string ENTER_PASSWORD = "Enter Password: ";
const string AUTH_FAILED = "Authentication failed. Disconnecting...\n";
const string WELCOME_MSG = "Welcome to the chat server!\n";
const string GROUP_EXISTS = "Error: Group already exists.\n";
const string GROUP_CREATED = "Group created successfully.\n";
const string USERNAME_EXISTS = "Error: Username already taken. Authentication failed. Disconnecting... \n";
const string INVALID_COMMAND = "Invalid command.\n";

// Data structures
unordered_map<int, string> clients;                    // Client socket -> username
unordered_map<string, string> users;                   // Username -> password
unordered_map<string, unordered_set<int>> groups;      // Group name -> set of client sockets
unordered_map<string,int> active_users;                    // Active usernames
mutex clients_mutex, groups_mutex, users_mutex, active_users_mutex, cout_mutex;

// Function to load users from a file
void load_users(const string& filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string username, password;
        getline(ss, username, ':');
        getline(ss, password);
        users[username] = password;
    }
}

// Function to send a message to a specific client
void send_message(int client_socket, const string& message) {
    send(client_socket, message.c_str(), message.size(), 0);
}

// Function to broadcast a message to all clients
void broadcast_message(const string& message, int sender_socket) {
    lock_guard<mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        if (client.first != sender_socket) {
            send_message(client.first, message);
        }
    }
}

// Function to handle a client's commands
void handle_client(int client_socket) {
    string username;

    // Authentication
    send_message(client_socket, ENTER_USERNAME);
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    username = buffer;

    

    

    send_message(client_socket, ENTER_PASSWORD);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, sizeof(buffer), 0);
    string password = buffer;

    {
        lock_guard<mutex> lock(users_mutex);
        if (users.find(username) == users.end()) {
            send_message(client_socket, AUTH_FAILED);
            close(client_socket);
            return;
        }
    }

    {
        lock_guard<mutex> lock(active_users_mutex);
        if (active_users.find(username) != active_users.end()) {
            send_message(client_socket, USERNAME_EXISTS);
            close(client_socket);
            return;
        }
        active_users[username]=client_socket; // Mark username as active
    }

    {
        lock_guard<mutex> lock(users_mutex);
        if (users[username] != password) {
            send_message(client_socket, AUTH_FAILED);
            {
                lock_guard<mutex> lock(active_users_mutex);
                active_users.erase(username); // Cleanup on failure
            }
            close(client_socket);
            return;
        }
    }

    // Welcome message
    {
        lock_guard<mutex> lock(clients_mutex);
        clients[client_socket] = username;
    }
    send_message(client_socket, WELCOME_MSG);
    broadcast_message(username + " has joined the chat.", client_socket);
    // Handle commands
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            broadcast_message(username + " has left the chat.", client_socket);
            {
                lock_guard<mutex> lock(active_users_mutex);
                active_users.erase(username);
            }
            {
                lock_guard<mutex> lock(clients_mutex);
                clients.erase(client_socket);
            }

            {
                lock_guard<mutex> lock(groups_mutex);
                // Remove the client from all groups
                for (auto it = groups.begin(); it != groups.end(); ) {
                    it->second.erase(client_socket);
                    
                    // If the group becomes empty, erase it
                    if (it->second.empty()) {
                        it = groups.erase(it);  // Erase and move iterator to the next valid position
                    } else {
                        ++it;
                    }
                }
            }
            close(client_socket);
            return;
        }

        string command = buffer;
        stringstream ss(command);
        string token;
        ss >> token;

        if (token == "/msg") {
            // cout << "AK" << endl;
            string target_username, message;
            ss >> target_username;
            getline(ss, message);

            if (target_username.empty() || message.empty()) {
                // cout << "HELLO" << endl;
                send_message(client_socket, INVALID_COMMAND);
            } else {
                lock_guard<mutex> lock(clients_mutex);
                if (active_users.find(target_username) != active_users.end()) {
                    int target_socket = active_users[target_username];
                    send_message(target_socket, "["+ username + "]" + message + "\n");
                    send_message(client_socket, "Message sent successfully.");
                    // send_message(client_socket, MESSAGE_SENT);
                } else {
                    send_message(client_socket, "There is no active user with this name");
                }
            }
        }
        else if (token == "/group_msg") {
            string group_name, message;
            ss >> group_name;
            getline(ss, message);

            if (group_name.empty() || message.empty()) {
                send_message(client_socket, INVALID_COMMAND);
            } else {
                lock_guard<mutex> lock(groups_mutex);
                if (groups.find(group_name) != groups.end()) {
                     if (groups[group_name].find(client_socket) == groups[group_name].end()) {
                        send_message(client_socket, "You are not a member of the group " + group_name + ".\n");
                        
                    }

                    else{
                    // Send the message to all group members except the sender
                    for (int member_socket : groups[group_name]) {
                        if (member_socket != client_socket) {
                            send_message(member_socket, "[" + username + " in " + group_name + "]: " + message + "\n");
                        }
                    }
                    send_message(client_socket, "Message sent to group " + group_name + ".\n");
                    }
                    
                } else {
                    send_message(client_socket, "Group does not exist.\n");
                }
            }
        }
        else if (token == "/broadcast") {
            string message;
            getline(ss, message);
            broadcast_message(username + ": " + message, client_socket);
            send_message(client_socket, "Message sent successfully.");
        } else if (token == "/create_group") {
            string group_name;
            ss >> group_name;

            if (group_name.empty()) {
                send_message(client_socket, INVALID_COMMAND);
            } else {
                lock_guard<mutex> lock(groups_mutex);
                if (groups.find(group_name) != groups.end()) {
                    send_message(client_socket, GROUP_EXISTS);
                } else {
                    groups[group_name].insert(client_socket);
                    send_message(client_socket, "Group " + group_name + " created.");
                }
            }
        } else if (token == "/join_group") {
            string group_name;
            ss >> group_name;

            lock_guard<mutex> lock(groups_mutex);
            if (groups.find(group_name) != groups.end()) {
                groups[group_name].insert(client_socket);
                send_message(client_socket, "Joined group " + group_name + ".\n");

                for (int member_socket : groups[group_name]) {
                    if (member_socket != client_socket) {
                        send_message(member_socket, "[" + group_name + "]: " + username + " has joined the group " + group_name + ".\n");
                    }
                }

            } else {
                send_message(client_socket, "Group does not exist.\n");
            }
        } 
        else if (token == "/leave_group") {
            string group_name;
            ss >> group_name;

            lock_guard<mutex> lock(groups_mutex);  // Locking the groups_mutex

            // Check if the group exists
            if (groups.find(group_name) != groups.end()) {
                // Check if the client is actually a member of the group
                if (groups[group_name].find(client_socket) == groups[group_name].end()) {
                    send_message(client_socket, "You are not a member of the group " + group_name + ".\n");
                    
                }

                // Remove the client from the group
                else{
                    groups[group_name].erase(client_socket);
                    
                    // If no members are left, remove the group
                    if (groups[group_name].empty()) {
                        groups.erase(group_name);
                    }

                    // Send confirmation message to the client
                    send_message(client_socket, "Left group " + group_name + ".\n");

                    // Send notification to other group members
                    for (int member_socket : groups[group_name]) {
                        if (member_socket != client_socket) {
                            send_message(member_socket, "[" + group_name + "]: " + username + " has left the group " + group_name + ".\n");
                        }
                    }
                }
            }
            else {
                send_message(client_socket, "Group does not exist.\n");
            }
        }   
        else {
            send_message(client_socket, INVALID_COMMAND);
        }
    }
}

// Main server function
int main() {
    load_users("users.txt");

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (::bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        cerr << "Error binding socket." << endl;
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        cerr << "Error listening on socket." << endl;
        return 1;
    }

    cout << "Server is listening on port 12345..." << endl;

    while (true) {
        sockaddr_in client_address{};
        socklen_t client_length = sizeof(client_address);
        int client_socket = accept(server_socket, (sockaddr*)&client_address, &client_length);
        if (client_socket < 0) {
            cerr << "Error accepting connection." << endl;
            continue;
        }

        thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    close(server_socket);
    return 0;
}
