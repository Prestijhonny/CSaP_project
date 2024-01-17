#!/bin/bash

cd ../
make
gnome-terminal -- bash -c "cd server/src;./server; exec bash"
gnome-terminal -- bash -c "cd client/src;./client < ../../startup/input1; exec bash"
gnome-terminal -- bash -c "cd client/src;./client < ../../startup/input2; exec bash"
