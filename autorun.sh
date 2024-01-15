#!/bin/bash

make
gnome-terminal -- bash -c "cd server/src;./server; exec bash"
gnome-terminal -- bash -c "cd client/src;./client << Ciao dal client 1; exec bash"
gnome-terminal -- bash -c "cd client/src;./client << Ciao dal client 2; exec bash"
