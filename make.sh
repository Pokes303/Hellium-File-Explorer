#!/bin/bash

echo "Building..."
clear
clear
make
read -n1 -p "Send rpx to console? (y/n): " answer
echo ""
if [ "$answer" = "y" ]; then
    echo "Sending via AppSender..."
    echo $SUDOPASS | sudo -S java -jar /opt/devkitpro/AppSender.jar ./Hellium-File-Explorer.rpx 192.168.1.44
fi