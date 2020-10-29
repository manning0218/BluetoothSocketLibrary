#include <algorithm>
#include <iostream>
#include <string>
#include <csignal>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>

#include "SDPService.hpp"
#include "BTSocket.hpp"

#define MAX_READ 1024*1024

std::unique_ptr<BTSocket> init_server(std::unique_ptr<BTSocket>& socket) {
    std::string service_name {"Manning BT Server"};
    std::string service_desc {"Bluetooth server for the raspberry pi in order to communicate with Android app"};
    std::string service_prov {"Manning Technologies"};

    SDPService sdpService;

    char buffer[1024] = {0};

    // register service
    auto session = sdpService.initializeService(service_name, service_desc, service_prov);
    if(sdpService.registerService(session, 11)) {
        std::cout << __FILE__ << ":" << __LINE__
            << " INFO: Successfully registered service.\n";
        socket->create();
        socket->bind(11);
        socket->listen();
        auto client = socket->accept();
        if(client) {
            auto addr = client->getSockAddr();
            ba2str(&addr->rc_bdaddr, buffer);
            std::cout << __FILE__ << ":" << __LINE__ 
                << " INFO: Accepted connection from client["
                << client->getSockFd() << "," << buffer
                << "]\n";
            return client;
        }
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to register service.\n";
        _exit(1);
    }

    return nullptr;
}

void printData(std::vector<unsigned char>& buffer) {
    std::string output;
    std::transform(buffer.begin(), buffer.end(), std::back_inserter(output),
            [](unsigned char c) { return char(c); });

    std::cout << __FILE__ << ":" << __LINE__
        << " INFO: Data received from client. buffer: "
        << output << "\n";
}

int main() {
    std::unique_ptr<BTSocket> server = std::make_unique<BTSocket>();
    auto client = init_server(server);
    std::vector<unsigned char> readBuffer(MAX_READ);
    std::vector<unsigned char> writeBuffer(500);

    std::string message {"I love you\r\n"};
    std::transform(message.begin(), message.end(), std::back_inserter(writeBuffer),
            [](char c) { return (unsigned char)(c); });

    if(client) {
        while(true) {
            int bytesAvailable = client->receive(readBuffer, MAX_READ, MSG_PEEK);
            if(bytesAvailable > 0) {
                auto bytesRead = client->receive(readBuffer, bytesAvailable);
                if(bytesRead > 0) {
                    printData(readBuffer);
                } 

                auto bytesWritten = client->send(writeBuffer);
                if(bytesWritten == int(writeBuffer.size())) {
                    std::cout << __FILE__ << ":" << __LINE__
                        << " TRACE: Successfully wrote message to client.\n";
                }

                if(client->disconnect()) {
                    std::cout << __FILE__ << ":" << __LINE__
                        << " INFO: Disconnected client from server. socket["
                        << client->getSockFd() << "]\n";
                }
            } else {
                std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR: Could not receive data off socket\n";
                break;
            }
        }
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failure in server initialization occurred.\n";
    }

    return 0;
}
