#! /usr/bin/bash

sudo mknod /dev/cosmos_rec c 100 0
sudo chmod 666 /dev/cosmos_rec

sudo mknod /dev/cosmos_fi c 101 0
sudo chmod 666 /dev/cosmos_fi

sudo mknod /dev/cosmos_ut c 102 0
sudo chmod 666 /dev/cosmos_ut
