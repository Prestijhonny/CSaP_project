#!/bin/bash

make
cd server/src
./server
sleep 3
cd client/src
./clie
