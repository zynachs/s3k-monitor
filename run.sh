#!/usr/bin/env bash

# Script to build and run s3k-monitor within a docker container 

function load_image() {
    if [ ! -f "$IMAGE.tgz" ]; then
        echo -e "\"$IMAGE.tgz\" does not exist in current directory."
        return 1
    fi
    docker load < $IMAGE.tgz
    return 0;
}

IMAGE=s3k-monitor-cc

docker inspect $IMAGE >/dev/null 2>&1

if [ $? -ne 0 ]; then
    echo -e "Docker image \"$IMAGE\" does not exist. Attempting to load it..."
    load_image
    [ $? -ne 0 ] && echo "Exiting."; exit
fi

uid=$(id -u)
gid=$(id -g)

docker run --rm -it \
    --user $uid:$gid \
    --mount type=bind,source="$(pwd)",target=/work/s3k-monitor \
    --mount type=bind,source="$(pwd)"/../s3k,target=/work/s3k \
    $IMAGE \
    qemu
