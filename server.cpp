#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm>
#include <ctime>

#define PORT 8080
#define BUFFER_SIZE 1024

struct Client {
    int socket;
    std::string username;
};

std::vector<Client> clients;

std::string getTimestamp() {
    time_t now = time(0);
    char* dt = ctime(&now);
    std::string ts(dt);
    ts.pop_back(); // remove newline
    return ts;
}

void broadcastMessage(const std::string &msg, int senderSocket) {
    for (auto &client : clients) {
        if (client.socket != senderSocket) {
            send(client.socket, msg.c_str(), msg.size(), 0);
        }
    }
}

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];

    // First message = username
    memset(buffer, 0, BUFFER_SIZE);
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    std::string username(buffer);

    clients.push_back({clientSocket, username});
    std::cout << username << " joined\n";

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytes <= 0) {
            std::cout << username << " disconnected\n";
            close(clientSocket);

            clients.erase(std::remove_if(clients.begin(), clients.end(),
                [clientSocket](Client c){ return c.socket == clientSocket; }),
                clients.end());
            break;
        }

        std::string msg(buffer);
        std::string formatted =
            "[" + getTimestamp() + "] " + username + ": " + msg;

        std::cout << formatted;
        broadcastMessage(formatted, clientSocket);
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "Server running on port " << PORT << "\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t len = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &len);
        std::cout << "Client connected\n";

        std::thread(handleClient, clientSocket).detach();
    }

    close(serverSocket);
}