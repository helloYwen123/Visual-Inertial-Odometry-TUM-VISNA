#!/usr/bin/env bash

REPO=nikolausdemmel/visnav_image

for t in 18.04 20.04 22.04; do
    docker build --pull -t $REPO:$t -f Dockerfile_$t .
done
