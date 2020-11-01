#include <iostream>

#include <errno.h>
#include <fcntl.h>

#include "BTSocket.hpp"
#include "SDPService.hpp"

BTSocket::BTSocket(int fd, struct sockaddr_rc* addr, socklen_t* addrLen) :
    sockFd_ {fd},
    addr_ {*addr},
    addrLen_ {*addrLen}
{}

int BTSocket::create(int sockFamily, int sockType, int protocol) {
    addr_.rc_family = sockFamily;
    addr_.rc_bdaddr = SDPService::bdAny;

    sockFd_ = ::socket(sockFamily, sockType, protocol);
    errNo_ = errno;
    if(sockFd_ == -1) {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to create socket.\n";
    }

    return sockFd_;
}

int BTSocket::bind(int port) {
    addr_.rc_channel = port;
    addrLen_ = sizeof(addr_);

    int result = ::bind(sockFd_, (struct sockaddr *)(&addr_), addrLen_);
    errNo_ = errno;
    if(result != 0) {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to bind to socket[" << sockFd_
            << "] on port[" << port << "]\n";
    }

    return result;
}

int BTSocket::listen(int backlog) {
    std::cout << __FILE__ << ":" << __LINE__ 
        << " TRACE: Listening for new connections. server["
        << sockFd_ << "]\n";
    int result = ::listen(sockFd_, backlog);
    errNo_ = errno;
    if(result != 0) {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to listen for socket[" << sockFd_
            << "]\n";
    }

    return result;
}

std::shared_ptr<BTSocket> BTSocket::accept() {
    std::shared_ptr<BTSocket> clientSocket {nullptr};
    struct sockaddr_rc peerAddr {0};
    socklen_t opt = sizeof(peerAddr);
    int clientFd = ::accept(sockFd_, (struct sockaddr *)(&peerAddr), &opt);
    errNo_ = errno;

    if(clientFd == -1) {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to accept client connection on socket["
            << sockFd_ << "]\n";
    } else {
        clientSocket = std::make_shared<BTSocket>(clientFd, &peerAddr, &opt);
    }

    return clientSocket;
}

bool BTSocket::setnonblock() {
    bool sockIsNonblocking {false};

    int flags = fcntl(sockFd_, F_GETFL, 0);
    errNo_ = errno;
    if(flags == -1) {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Faield to get current lock on socket["
            << sockFd_ << "]\n";
    } else {
        int result = fcntl(sockFd_, F_SETFL, O_NONBLOCK);
        errNo_ = errno;
        if(result == -1) {
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Failed to set socket[" << sockFd_ 
                << "] in nonblocking mode.\n";
        }

        sockIsNonblocking = result != -1;
    }

    return sockIsNonblocking;
}

int BTSocket::receive(std::vector<char>& buffer, int len, int flags) {
    int bytesRead {-1};
    if(len > 0) {
        bytesRead = recv(sockFd_, buffer.data(), len, flags);
        errNo_ = errno;

        if(bytesRead == -1) {
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Failed to read data from socket["
                << sockFd_ << "]\n";
        } else if(bytesRead == 0) {
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Zero read on socket[" 
                << sockFd_ << "]\n";
            disconnect();
        } 
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Invalid length to read set for socket["
            << sockFd_ << "] len[" << len << "]\n";
    }

    return bytesRead;
}

int BTSocket::send(const std::vector<unsigned char>& buffer, int flags) {
    int bytesWritten {-1};
    if(!buffer.empty()) {
        bytesWritten = ::send(sockFd_, buffer.data(), buffer.size(), flags);
        errNo_ == errno;
        if(bytesWritten == -1) {
            std::cout << __FILE__ << ":" << __LINE__
                << " ERROR: Failed to write to socket["
                << sockFd_ << "]\n";
        }
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Sending empty data for socket["
            << sockFd_ << "]\n";
    }

    return bytesWritten;
}

bool BTSocket::disconnect() {
    int retVal = close(sockFd_);
    errNo_ = errno;

    if(retVal == -1) {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to close the socket[" 
            << sockFd_ << "].\n";
    }

    return retVal == 0;
}
