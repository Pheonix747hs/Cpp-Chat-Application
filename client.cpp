#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void receiveMessages(int sock) {
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock, buffer, BUFFER_SIZE, 0);

        if (bytes <= 0) {
            std::cout << "Disconnected from server\n";
            break;
        }

        std::cout << buffer;
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Connection failed\n";
        return 1;
    }

    std::string username;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    username += "\n";
    send(sock, username.c_str(), username.size(), 0);

    std::cout << "Connected to server\n";

    std::thread(receiveMessages, sock).detach();

    std::string msg;
    while (true) {
        std::getline(std::cin, msg);
        msg += "\n";
        send(sock, msg.c_str(), msg.size(), 0);
    }

    close(sock);
}