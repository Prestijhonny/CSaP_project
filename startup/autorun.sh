#!/bin/bash

make
gnome-terminal -- bash -c "cd server/src;./server; exec bash"
gnome-terminal -- bash -c "cd client/src;echo -e "Ciao dal client 1!" | ./client; exec bash"
gnome-terminal -- bash -c "cd client/src;echo -e "Ciao dal client 2!" | ./client; exec bash"
