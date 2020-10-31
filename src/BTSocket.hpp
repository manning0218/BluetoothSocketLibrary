#ifndef BT_SOCKET_HPP
#define BT_SOCKET_HPP

#include <memory>
#include <vector>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

class BTSocket {
    public:
        BTSocket() = default;
        BTSocket(int fd, struct sockaddr_rc* addr);
        ~BTSocket() = default;

        // TODO: Add support for both move and copy operations
        // of the socket
        //BTSocket(BTSocket&& btSocket);
        //BTSocket(const BTSocket& btSocket);

        //BTSocket &=operator(BTSocket&& btSocket);
        //BTSocket &=operator(const BTSocket& btSocket);


        // Required socket configuration
        int create(int sockFamily = AF_BLUETOOTH, int sockType = SOCK_STREAM, int protocol = BTPROTO_RFCOMM);
        int bind(int port);
        int listen(int backlog = MAX_CONN_QUEUE);
        std::unique_ptr<BTSocket> accept(); // Used for servers

        // Optional socket configuration
        int setnonblock();

        // Send and receive data on connected socket
        int receive(std::vector<unsigned char>& buffer, int len, int flags = 0);
        int send(const std::vector<unsigned char>& buffer, int flags = 0);

        // Disconnect calls
        bool disconnect();
        struct sockaddr_rc* getSockAddr() { return &addr_; }
        int getSockFd() { return sockFd_; }
        int getErrorNo() { return errNo_; }

    private:
        int sockFd_;
        struct sockaddr_rc addr_ {0};
        socklen_t addrLen_;

        int bytesOnSocket_;

        int errNo_;
        
        static constexpr const int MAX_CONN_QUEUE = 10;
};

#endif
