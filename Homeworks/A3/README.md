
#  TCP Handshake Client (Raw Sockets)

##  Description

This project implements the **client-side of a TCP 3-way handshake** using **raw sockets** in C++.  
The client **manually constructs** and sends IP and TCP headers to establish a connection with a server by performing the following steps:

1. Send **SYN** to initiate connection.  
2. Receive **SYN-ACK** from the server.  
3. Send **ACK** to complete the handshake.

All packets are crafted manually, **bypassing the OS's built-in TCP stack**.
##  How It Works

### Step-by-step:

1. **Creates a raw socket** using `SOCK_RAW` and `IPPROTO_TCP`.
2. **Sets the `IP_HDRINCL` option** to tell the OS that IP headers will be provided manually.
3. **Constructs a SYN packet** with a custom sequence number.
4. **Listens for a SYN-ACK** response from the server.
5. **Sends the final ACK** to complete the handshake.

### Key Points:
- The client communicates directly with the server at **127.0.0.1** (localhost) and port **12345**. The client uses port **54321**.
- The TCP handshake is manually simulated with crafted IP and TCP headers, allowing for control over the packet content.

---
###  Packet Fields

#### IP Header:
- **Version:** IPv4  
- **TTL:** 64  
- **Total Length:** Includes both IP and TCP headers  

#### TCP Header:
- **Source Port:** 54321  
- **Destination Port:** 12345  
- **Sequence Number / Acknowledgment Number**  
- **Flags:** `SYN`, `ACK`  
- **Window Size:** 8192  
- **Checksum** 

##  Overview of the 3-Way Handshake

The TCP 3-way handshake is manually implemented with the following custom sequence and acknowledgment numbers:


- Client → Server: SYN with seq = 200
- Server → Client: SYN-ACK with seq = 400, ack = 201
- Client → Server: ACK with seq = 600, ack = 401

This illustrates a complete low-level implementation of a TCP handshake using raw sockets.



---

## Files implemented:

- `client.cpp` – Main source file of client containing raw socket code for the TCP handshake.

---

## Requirements

- Linux-based system  
- `g++` compiler  
- **Root privileges** (required for raw socket usage)

---

## Compilation and Execution

1. **Compile the program**:
    ```bash
    g++ client.cpp -o client
    ```

2. **Run the program with root privileges**:
    ```bash
    sudo ./client
    ```
 - The client will initiate the connection and go through the 3-way handshake. Make sure the server is running before starting the client.
## Individual Contributions

### Abhishek Khandelwal

### Poojal Katiyar

### Pallav Goyal

All of the three members contributed equally into the assignment.

