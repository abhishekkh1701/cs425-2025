# Chat Server

## Features

### Implemented Features

1. **Authentication**:
   - Users must provide a valid username and password to authenticate and connect to the chat server.  

2. **Private Messaging**:
   - Clients can send private messages to other active users using the `/msg` command.

3. **Group Messaging**:
   - Users can create groups, join existing groups, and send messages to all members of a group using `/group_msg`.
4. **Broadcast Messaging**:
   - Messages can be broadcasted to all connected users using `/broadcast`.
5. **Group Management**:
   - Users can create, join, and leave groups using the respective commands `/create_group`, `/join_group`, and `/leave_group`.
6. **Dynamic User Addition**:
   - Users can join and leave groups as needed, and the server dynamically manages group membership.
7. **User List Management**:
   - A user can only be logged in from one session at a time.

### Not Implemented Features

1. **File Transfer**: The server does not support file transfer functionality.
2. **User Profile Management**: There is no profile management or ability to update user information.
3. **Persistent Chat History**: Chat history is not saved on the server. Once the client disconnects, the history is lost.

## Design Decisions

### 1. **Multithreading for Client Handling**
   - **Decision:** A new thread is created for each client connection using `std::thread` and `detach()`. This allows the server to handle multiple clients concurrently without blocking other connections.    

### 2. **Handling Client Disconnection**
   - **Decision:** When a client disconnects (Ctrl+C or network failure), `recv()` returning `0` or `-1` triggers cleanup and afterwards ensuring proper resource management by removing the user from `clients`, `active_users`, and all joined groups.  
  

### 3. **Group Membership Management**
   - **Decision:** When a client disconnects, they are removed from all groups. If a group becomes empty, it is deleted. In our implementation, group members are only active users, if a disconnected client later connects and still wants to send the message to the group he was part of earlier, he has to rejoin the group, (if it shall be still existing) and then send the message.

### 4. **Synchronization Using Mutex Locks**
   - **Decision:** Used `std::mutex` (`lock_guard`) to synchronize access to shared resources namely (`clients`, `groups`, `active_users`), depending on which resouce is being accessed .  It helps preventing race conditions when multiple clients access or modify shared data.  

### 5. **Use of Maps**
   - **Decision:** Different types of maps are used for efficient communication:
  - `clients` (maps client sockets to usernames) for tracking connected users.
  - `active_users` (maps usernames to client sockets) for efficient private messaging.
  - `groups` (maps group names to sets of client sockets) for group-based messaging.
  - `users` for mapping usernames with password useful in authentication.

### 6. **Username and Group Uniqueness**
   - **Decision:** Users cannot have duplicate usernames, and groups must have unique names. 

### 7. **Other Points**
   - **Decision:**  - Use of broadcast messaging to notify active clients whenever a new client joins the chat and active users of groups when a new user joins the corresponding group.
   - We also mention the person in group who sends a group message to group members (a bit different to what was shown in example image). For example `[Alice in CS425]`.
   - We used different types of error messages depending on the situation.

## Implementation

### **1. Introduction**

The server is a multi-threaded application designed to handle multiple client connections concurrently. It allows users to log in, send private messages, create/join/leave groups, send messages to groups, and broadcast messages to all connected clients.

### **2. Server Setup**

The server begins by creating a socket and binding it to a specific port (`12345`). It listens for incoming client connections in an infinite loop. When a client connects, the server accepts the connection and spawns a new thread to handle the client's communication. This allows the server to manage multiple clients at once without blocking.

### **3. User Authentication**

Upon client connection, the server requests the username and password. It checks the username against a pre-stored list (loaded from a file `users.txt`) and verifies if the password matches the stored one. If the authentication fails, the server sends an "Authentication failed" message and disconnects the client.

### **4. Multi-Threading for Client Handling**

Each client is handled in a separate thread to ensure that the server can handle multiple clients concurrently without blocking. This approach uses `std::thread` to create a new thread for each client. The server also uses `thread::detach()` to allow the thread to run independently, ensuring that the main server thread remains free to accept new connections.

### **5. Handling Client Commands**

After authentication, the server waits for and processes various client commands:
- **Private Messaging (`/msg`)**: Allows a client to send a private message to another client. The server checks if the target user is online and forwards the message if they are.
- **Group Messaging (`/group_msg`)**: Clients can send messages to a specific group. The server ensures the sender is a member of the group and broadcasts the message to all group members except the sender.
- **Broadcast Messaging (`/broadcast`)**: Allows the client to send a message to all connected clients. The server forwards the message to every client, except the sender.
- **Create Group (`/create_group`)**: Clients can create a new group. If the group name already exists, the server sends an error message.
- **Join Group (`/join_group`)**: A client can join an existing group. The server checks if the group exists and adds the client to the group.
- **Leave Group (`/leave_group`)**: A client can leave a group. If the group becomes empty after a client leaves, it is deleted.

### **6. Group Membership Management**

Groups are stored in a map where each group name is associated with a set of client sockets. When a client joins or leaves a group, the server updates this map accordingly. If a group becomes empty after a client leaves, the group is deleted. 

Additionally, when a client disconnects, they are automatically removed from all groups they were a part of. If the group is empty after the client leaves, it is also removed from the server’s memory.

### **7. Synchronization Using Mutex Locks**

Because the server uses shared data structures (like the map for clients, active users, and groups), synchronization is crucial to avoid race conditions. The server employs `std::mutex` and `lock_guard` to ensure that only one thread can modify shared data at a time. Mutex locks are used to synchronize access to critical sections, such as the list of active users and groups.

- **clients_mutex**: Ensures safe access to the `clients` map, which associates client sockets with usernames.
- **groups_mutex**: Ensures safe modification of the `groups` map, which associates group names with sets of client sockets.
- **active_users_mutex**: Protects the `active_users` map, which tracks which users are currently logged in.

### **8. Clean-Up and Client Disconnection**

When a client disconnects (either voluntarily or due to network failure), the server performs a clean-up process:
1. The client is removed from the `active_users`, `clients`, and any groups they are part of.
2. If the group becomes empty, the server deletes the group.
3. A message is broadcast to notify other clients that the user has left the chat.
4. The server closes the client’s socket.

### **9. Error Handling and Invalid Commands**

The server performs error handling for invalid commands. If a client sends a command that is not recognized (such as `/unknown_command`), the server sends an "Invalid command" message. Additionally, if a client tries to join a group which does not exist or sends a message to a user who is not online, the server will send an appropriate error message.

### **10. Server Shutdown**

The server runs indefinitely, accepting new connections and handling client requests. It will only shut down when the main process is terminated (e.g., by sending a termination signal like Ctrl+C). The server closes all client connections before exiting.

---

This implementation is designed to be scalable and responsive, handling multiple clients simultaneously while ensuring thread safety and data integrity through synchronization. The use of threads allows each client to interact with the server independently without blocking other clients, making the system suitable for real-time chat applications.

### Code Flow

1. **Client Connection**:
   - The server accepts a connection and creates a new thread for the client.
2. **Authentication**:
   - The client sends their username and password. The server verifies them against the stored user data.
3. **Command Handling**:
   - Once authenticated, the client can send commands such as `/msg`, `/group_msg`, `/create_group`, etc.
4. **Client Disconnection**:
   - Upon disconnection, the server cleans up the client's data and broadcasts their departure.

## Testing

### Types of Testing Performed

1. **Correctness Testing**:
   - Valid and invalid usernames and passwords were tested.
   - Different command types were tested (private messages, group messages, broadcast messages).
   - Group membership functionality was verified.

2. **Stress Testing**:
   - The server was tested with multiple clients(20,30) simultaneously using scripts to ensure it could handle a large number of users without crashing. 

### Challenges

1. **Thread Synchronization**: Initially, race conditions were occurring when multiple clients tried to access shared data. This was resolved by using mutexes (`std::mutex`).
2. **Group Management**: Handling clients joining and leaving groups required careful synchronization to avoid errors when managing group memberships.
3. **Socket Communication**: Ensuring the server correctly handled socket disconnections and client cleanup was challenging, requiring extensive error checking.

## Restrictions

1. **Maximum Clients**: The number of clients that can connect is limited by system resources (e.g., file descriptors).
2. **Maximum Group Size**: There is no explicit limit on the number of members in a group, but practical constraints are imposed by memory and performance.
3. **Message Size**: The maximum message size is limited by the buffer size (1024 bytes). Larger messages will be truncated.

## Individual Contributions

### Abhishek Khandelwal (220040)

### Poojal Katiyar (220770)

### Pallav Goyal (220747)

All of the three members contributed equally into the assignment.

## Sources

- C++ Documentation
- Online forums and blogs (StackOverflow, C++ reference)
- Tutorials on multithreading and socket programming in C++

## Feedback

- We liked the assignment.
