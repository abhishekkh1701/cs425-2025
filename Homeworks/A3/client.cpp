#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define CLIENT_PORT 54321

// Function to calculate checksum
unsigned short checksum(void *b, int len) {
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// Function to send a TCP packet
void send_tcp_packet(int sock, struct sockaddr_in *dest_addr, int seq, int ack, int syn, int ack_flag) {
    char packet[sizeof(struct iphdr) + sizeof(struct tcphdr)];
    memset(packet, 0, sizeof(packet));

    struct iphdr *ip = (struct iphdr *)packet;
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct iphdr));

    // Fill IP header
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(packet));
    ip->id = htons(54321);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = inet_addr("127.0.0.1");
    ip->daddr = dest_addr->sin_addr.s_addr;
    ip->check = checksum(ip, sizeof(struct iphdr));

    // Fill TCP header
    tcp->source = htons(CLIENT_PORT);
    tcp->dest = htons(SERVER_PORT);
    tcp->seq = htonl(seq);
    tcp->ack_seq = htonl(ack);
    tcp->doff = 5;
    tcp->syn = syn;
    tcp->ack = ack_flag;
    tcp->window = htons(8192);
    tcp->check = 0;
    tcp->check = checksum(tcp, sizeof(struct tcphdr));

    // Send packet
    if (sendto(sock, packet, sizeof(packet), 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr)) < 0) {
        perror("sendto() failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Step 1: Send SYN (Seq=200)
    std::cout << "[+] Sending SYN..." << std::endl;
    send_tcp_packet(sock, &server_addr, 200, 0, 1, 0);

    char buffer[65536];
    struct sockaddr_in source_addr;
    socklen_t addr_len = sizeof(source_addr);

    // Step 2: Receive SYN-ACK
    while (true) {
        int data_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&source_addr, &addr_len);
        if (data_size < 0) {
            perror("recvfrom() failed");
            continue;
        }

        struct iphdr *ip = (struct iphdr *)buffer;
        struct tcphdr *tcp = (struct tcphdr *)(buffer + (ip->ihl * 4));

        if (ntohs(tcp->source) == SERVER_PORT && tcp->syn == 1 && tcp->ack == 1 && ntohl(tcp->seq) == 400) {
            std::cout << "[+] Received SYN-ACK..." << std::endl;
            break;
        }
    }

    // Step 3: Send ACK (Seq=600, Ack=401)
    std::cout << "[+] Sending ACK..." << std::endl;
    send_tcp_packet(sock, &server_addr, 600, 401, 0, 1);

    std::cout << "[+] Handshake complete!" << std::endl;
    close(sock);
    return 0;
}
