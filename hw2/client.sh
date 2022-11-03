#!/bin/bash

rm -rf client
make client
./client $1 127.0.0.1:7878
