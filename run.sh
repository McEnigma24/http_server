#!/bin/bash

source config

clear

# echo "Compiling...."
# [ -f out ] && rm -f out
# g++ -std=c++20 *.cpp *.hpp *.h -w -o out


# # clear

# ./out

# echo -e "\n\n\n"


# libgpiod-dev
pip install pyphen

create_dir $DIR_BUILD

cmake -S . -B $DIR_BUILD;
cmake --build $DIR_BUILD;

cd $DIR_BUILD;

clear
sudo setcap 'cap_net_bind_service=+ep' HTTP_SERVER.exe
./HTTP_SERVER.exe