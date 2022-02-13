#!/bin/bash

echo "Building..."
clear
clear
make
read -n1 -p "Send rpx to console? (y/n): " answer
if [ "$answer" = "y" ]; then
    echo "Sending via AppSender..."
    java -jar /opt/devkitpro/AppSender.jar ./Hellium-File-Explorer.rpx 192.168.1.44 
else
    echo ""
fi