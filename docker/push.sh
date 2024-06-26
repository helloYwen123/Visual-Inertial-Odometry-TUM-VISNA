#!/usr/bin/env bash

REPO=nikolausdemmel/visnav_image

docker login

for t in 18.04 20.04 22.04; do
    docker push $REPO:$t
done
