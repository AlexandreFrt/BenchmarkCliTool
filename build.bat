@echo off
if not exist bin mkdir bin
g++ .\main.cpp -o .\bin\main.exe -O0 -march=native -pthread -std=c++17