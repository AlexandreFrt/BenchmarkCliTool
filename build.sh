#!/bin/bash
mkdir -p bin
g++ main.cpp -o bin/main.elf -O0 -march=native -pthread -std=c++17