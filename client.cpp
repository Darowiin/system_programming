#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#define BUFFER_SIZE 1024

// ANSI color codes
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";

void print_banner() {
    std::cout << MAGENTA << R"(
  _____                     _   _
 / ____|                   | | (_)
| |  __  __ _ _ __ ___   __| |  _ _ __   __ _
| | |_ |/ _` | '_ ` _ \ / _` | | | '_ \ / _` |
| |__| | (_| | | | | | | (_| | | | | | | (_| |
 \_____|\__,_|_| |_| |_|\__,_| |_|_| |_|\__, |
                                         __/ |
                                        |___/ )" << RESET << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << RED << "Usage: " << argv[0] << " <server_ip> <port>" << RESET << std::endl;
        return 1;
    }
    const char* ip = argv[1];
    int port = std::atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        std::cerr << RED << "Invalid port number." << RESET << std::endl;
        return 1;
    }

    print_banner();
    std::cout << BLUE << "Connecting to " << ip << ":" << port << "..." << RESET;
    std::cout << std::flush;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    connect(sock, (sockaddr*)&addr, sizeof(addr));

    std::cout << GREEN << " Connected!" << RESET << std::endl;

    char buf[BUFFER_SIZE];
    ssize_t n = recv(sock, buf, sizeof(buf)-1, 0);
    if (n > 0) {
        buf[n] = '\0';
        std::cout << CYAN << buf << RESET;
    }

    while (true) {
        std::cout << YELLOW << "\nYour guess (1-100): " << RESET << std::flush;
        std::string guess;
        if (!std::getline(std::cin, guess) || guess.empty()) continue;
        send(sock, guess.c_str(), guess.size(), 0);

        ssize_t len = recv(sock, buf, sizeof(buf)-1, 0);
        if (len <= 0) break;
        buf[len] = '\0';
        std::string resp(buf);
        if (resp.find("Too low") != std::string::npos || resp.find("Too high") != std::string::npos)
            std::cout << RED << resp << RESET;
        else if (resp.find("Correct") != std::string::npos) {
            std::cout << GREEN << resp << " You win!" << RESET << std::endl;
            break;
        } else
            std::cout << resp;
    }
    close(sock);
    return 0;
}
