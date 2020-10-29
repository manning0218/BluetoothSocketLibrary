#!/bin/bash
if [ $1 == "realease" ]; then
    g++ -O3 -std=c++17 -I ./ -c SDPService.cpp -o SDPService.o
    g++ -O3 -std=c++17 -I ./ -c BTSocket.cpp -o BTSocket.o
    g++ -O3 -std=c++17 -I ./ -c main.cpp -o main.o
    
    g++ -O3 -std=c++17 -I ./ main.o SDPService.o BTSocket.o -o bluetooth_conn_service_release -lbluetooth 
elif [ $1 == "debug" ]; then
    g++ -g -O0 -std=c++17 -I ./ -c SDPService.cpp -o SDPService.o
    g++ -g -O0 -std=c++17 -I ./ -c BTSocket.cpp -o BTSocket.o
    g++ -g -O0 -std=c++17 -I ./ -c main.cpp -o main.o
            
    g++ -g -O0 -std=c++17 -I ./ main.o SDPService.o BTSocket.o -o bluetooth_conn_service_debug -lbluetooth
fi
