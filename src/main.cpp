#include <iostream>
#include <string>
#include <csignal>

#include <unistd.h>
#include <sys/socket.h>

#include "SDPService.hpp"

int init_server() {
    std::string service_name {"Manning BT Server"};
    std::string service_desc {"Bluetooth server for the raspberry pi in order to communicate with Android app"};
    std::string service_prov {"Manning Technologies"};

    SDPService sdpService;

    int result, sock, client;
    uint8_t port {11};
    struct sockaddr_rc loc_addr {0}, rem_addr {0};
    socklen_t opt = sizeof(rem_addr);

    char buffer[1024] = {0};

    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = SDPService::bdAny;
    loc_addr.rc_channel = port;

    // register service
    auto session = sdpService.initializeService(service_name, service_desc, service_prov);
    if(sdpService.registerService(session, port)) {
        std::cout << __FILE__ << ":" << __LINE__
            << " INFO: Successfully registered service.\n";
    } else {
        std::cout << __FILE__ << ":" << __LINE__
            << " ERROR: Failed to register service.\n";
        _exit(1);
    }

    // allocate socket
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    std::cout << "socket() returned: " << sock << "\n";

    // bind socket to port 3 
    result = bind(sock, (struct sockaddr *)(&loc_addr), sizeof(loc_addr));
    std::cout << "bind() returned: " << result << "\n";

    // start listening for connections
    result = listen(sock, 1);
    std::cout << "listen() returned: " << result << "\n";

    // accept the connection
    client = accept(sock, (struct sockaddr *)(&rem_addr), &opt);
    std::cout << "accept() returned: " << client << "\n";

    ba2str(&rem_addr.rc_bdaddr, buffer);
    std::cout << "Accepted connection from " << buffer << "\n";
    memset(buffer, 0, sizeof(buffer));

    return client;
}

bool read_server(int client) {
    char input[1024] = {'\0'};
    int bytes_read = read(client, input, sizeof(input));

    if(bytes_read > 0) {
        std::cout << "Received: " << input << "\n";
        return true;
    } else {
        return false;
    }
} 

bool write_server(int client) {
    char message[1024] = "";
    std::cout << "Enter message to send: ";
    std::cin >> message;
    
    std::string messageStr(message);
    std::cout << "Message to send: " << messageStr << "\n";
    int bytes_sent {0};
    size_t size {sizeof(message)/sizeof(char)};
    std::cout << "Message size: " << size << "\n";
    int remaining {size};

    
    while(remaining > 0) {
        std::cout << "Sending at offset: " << size - remaining << "\n";
        bytes_sent = send(client, &message[size - remaining], size, 0);
        if(bytes_sent >= 0) {
            std::cout << "Bytes Sent: " << bytes_sent << "\n";
            remaining -= bytes_sent;
            std::cout << "Remaining to send: " << remaining << "\n";
        } else {
            return false;
        }
    }

    bytes_sent = send(client, "\n", 1, 0);

    return true;
}

int main() {
    int client_fd = init_server();

    while(true) {
        if(!read_server(client_fd)) break;
        if(!write_server(client_fd)) break;
    }

    return 0;
}
