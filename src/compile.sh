#!/bin/bash
g++ -O3 -std=c++17 -I ./ -c SDPService.cpp -o SDPService.o
g++ -O3 -std=c++17 -I ./ -c main.cpp -o main.o
