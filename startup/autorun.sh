#!/bin/bash

make
gnome-terminal -- bash -c "cd server/src;./server; exec bash"
gnome-terminal -- bash -c "cd client/src;./client < input1; exec bash"
gnome-terminal -- bash -c "cd client/src;./client < input2; exec bash"
