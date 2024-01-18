#!/bin/bash

make
gnome-terminal -- bash -c "cd server/src;./server; exec bash"
gnome-terminal -- bash -c "cd client/src;./client; exec bash"
gnome-terminal -- bash -c "cd client/src;./client; exec bash"