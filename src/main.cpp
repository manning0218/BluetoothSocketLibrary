#include <algorithm>
#include <iostream>
#include <string>
#include <csignal>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>

#include <string.h>

#include <unistd.h>
#include <sys/socket.h>

#include "SDPService.hpp"
#include "BTSocket.hpp"
#include "EventManager.hpp"

#define MAX_READ 1024*1024

std::unordered_map<int, std::shared_ptr<BTSocket>> clients;

bool init_server(std::shared_ptr<BTSocket>& socket) {
    std::string service_name {"Manning BT Server"};
    std::string service_desc {"Bluetooth server for the raspberry pi in order to communicate with Android app"};
    std::string service_prov {"Manning Technologies"};

    SDPService sdpService;

    bool serverInitialized {false};

    // register service
    auto session = sdpService.initializeService(service_name, service_desc, service_prov);
    if(sdpService.registerService(session, 11)) {
        std::cout << __FILE__ << ":" << __LINE__
            << " INFO: Successfully registered service.\n";
        socket->create();
        socket->bind(11);
        socket->listen();

        socket->setnonblock();
        serverInitialized = true;
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to register service.\n";
        _exit(1);
    }

    return serverInitialized;
}

void printData(std::vector<char>& buffer) {
    std::string output;
    std::transform(buffer.begin(), buffer.end(), std::back_inserter(output),
            [](char c) { return char(c); });

    std::cout << __FILE__ << ":" << __LINE__
        << " INFO: Data received from client. buffer: "
        << output << "\n";
}

void readClient(std::shared_ptr<BTSocket> client) {
    std::vector<char> buffer(MAX_READ);
    if(client) {
        auto clientFd = client->getSockFd();
        int bytesAvailable = client->receive(buffer, MAX_READ, MSG_PEEK);
        if(bytesAvailable > 0) {
            int bytesRead = client->receive(buffer, bytesAvailable);
            if(bytesRead == -1) {
                std::cout << __FILE__ << ":" << __LINE__
                    << " ERROR: Client failed to read available bytes. client["
                    << client->getSockFd() << "] errno[" << client->getErrorNo() 
                    << "] reason[" << strerror(client->getErrorNo()) << "]\n";
                if(EventManager::deregisterEvent(clientFd)) {
                    std::cout << __FILE__ << ":" << __LINE__
                        << " INFO: Deregistered client from event manager. client["
                        << clientFd << "]\n";
                    clients.erase(clientFd);
                }
            } else {
                printData(buffer);
            }
        } else {
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Client read failed to see how much data is available to read."
                << " client[" << client->getSockFd() << "] errno[" << client->getErrorNo()
                << "] reason[" << strerror(client->getErrorNo()) << "]\n";
            if(EventManager::deregisterEvent(clientFd)) {
                std::cout << __FILE__ << ":" << __LINE__
                    << " INFO: Deregistered client from event manager. client["
                    << clientFd << "]\n";
                clients.erase(clientFd);
            }
        }
    }
}

bool addReadHandle(std::shared_ptr<BTSocket> client) {
    bool addedReadHandle {false};
    
    if(client) {
        auto clientHandler = std::make_unique<EventHandlerImpl<void>>();
        clientHandler->fd_ = client->getSockFd();
        clientHandler->func_ = std::bind(readClient, client);
        clientHandler->event_ = EPOLLIN | EPOLLET;
        if(EventManager::registerEvent(std::move(clientHandler))) {
            std::cout << __FILE__ << ":" << __LINE__
                << " INFO: Registered read event for client["
                << client->getSockFd() << "]\n";
            addedReadHandle = true;
        }
    }

    return addedReadHandle;
}

void addNewClient(std::shared_ptr<BTSocket> server) {
    if(server) {
        auto client = server->accept();
        clients.emplace(client->getSockFd(), client);
        if(!addReadHandle(client)) {
           std::cout << __FILE__ << ":" << __LINE__
               << " ERROR: Failed to add client read event. client["
               << client->getSockFd() << "]\n";
        } 
    } 
}

bool addListenHandler(std::shared_ptr<BTSocket> server) {
    bool addedListenerHandle {false};
    if(server) {
        auto serverHandler = std::make_unique<EventHandlerImpl<void>>();
        serverHandler->fd_ = server->getSockFd();
        serverHandler->func_ = std::bind(addNewClient, server);
        serverHandler->event_ = EPOLLIN;
        if(EventManager::registerEvent(std::move(serverHandler))) {
            std::cout << __FILE__ << ":" << __LINE__
                << " INFO: Registered listen event to event manager. socket["
                << server->getSockFd() << "]\n";
            addedListenerHandle = true;
        }
    }

    return addedListenerHandle;
}


void run(std::shared_ptr<BTSocket> server) {
    if(EventManager::initialize()) {
        if(addListenHandler(server)) {
            std::cout << __FILE__ << ":" << __LINE__
                << " INFO: Added listener to epoll instance. socket["
                << server->getSockFd() << "]\n";

            EventManager::run();        
        }
    }
}

int main() {
    std::shared_ptr<BTSocket> server = std::make_shared<BTSocket>();
    if(init_server(server)) {
       run(server); 
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failure in server initialization occurred.\n";
    }

    return 0;
}
