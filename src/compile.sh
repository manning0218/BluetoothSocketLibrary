#!/bin/bash

if [ ! -d "./build/bin" ]; then
    mkdir -p ./build/bin
fi

if [ $1 == "realease" ]; then
    if [ ! -d "./build/release/obj" ]; then
        mkdir -p ./build/release/obj
    fi

    g++ -O3 -std=c++17 -I ./ -c SDPService.cpp -o build/release/obj/SDPService.o
    g++ -O3 -std=c++17 -I ./ -c BTSocket.cpp -o build/release/obj/BTSocket.o
    g++ -O3 -std=c++17 -I ./ -c EventManager.cpp -o build/release/obj/EventManager.o
    g++ -O3 -std=c++17 -I ./ -c main.cpp -o build/release/obj/main.o

    objs=$(find build/release/obj/*)
    g++ -O3 -std=c++17 -I ./ $objs -o build/bin/bluetooth_conn_service_release -lbluetooth
elif [ $1 == "debug" ]; then
    if [ ! -d "./build/debug/obj" ]; then
        mkdir -p "./build/debug/obj"
    fi

    g++ -g -O0 -std=c++17 -I ./ -c SDPService.cpp -o build/debug/obj/SDPService.o
    g++ -g -O0 -std=c++17 -I ./ -c BTSocket.cpp -o build/debug/obj/BTSocket.o
    g++ -g -O0 -std=c++17 -I ./ -c EventManager.cpp -o build/debug/obj/EventManager.o
    g++ -g -O0 -std=c++17 -I ./ -c main.cpp -o build/debug/obj/main.o

    objs=$(find build/debug/obj/*)
    g++ -g -O0 -std=c++17 -I ./ $objs -o build/bin/bluetooth_conn_service_debug -lbluetooth
elif [ $1 == "clean" ]; then 
    rm -rf ./build/release/obj/
    rm -rf ./build/debug/obj
    rm -rf ./build/bin
fi
