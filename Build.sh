#!/bin/bash
# Name: Hoikin Cheng

# Builds both Server.cpp and Retriever.cpp

g++ Server.cpp -o Server -pthread
g++ Retriever.cpp -o Retriever
