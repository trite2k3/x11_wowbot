#!/bin/bash


# build record
g++ -std=c++17 -O2 record_path.cpp -lX11 -o record_path

# build bot
g++ read_coords.cpp -o read_coords -lX11 -lXtst


