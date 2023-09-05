#!/usr/bin/env bash

# Script to build and run s3k-monitor within a docker container 

IMAGE=ghcr.io/zynachs/s3k-monitor-cc:latest

docker inspect $IMAGE >/dev/null 2>&1

if [ $? -ne 0 ]; then
    echo -e "Docker image \"$IMAGE\" does not exist in local registry. Pulling from internet..."
    docker pull $IMAGE 
    if [ $? -ne 0 ]; then 
        echo "Failed to pull image.";
        exit
    fi
fi

if [ $# -eq 0 ]; then
    COMMAND="qemu"
else
    COMMAND="$1"
fi

uid=$(id -u)
gid=$(id -g)

docker run --rm -it \
    --user $uid:$gid \
    --mount type=bind,source="$(pwd)",target=/work/s3k-monitor \
    --mount type=bind,source="$(pwd)"/../s3k,target=/work/s3k \
    $IMAGE \
    $COMMAND
