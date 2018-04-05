#!/bin/sh

VERSION=$1

docker build -t openkorp/opendlv-device-ps3controller-aarch64:$VERSION -f Dockerfile.aarch64 . &
docker build -t openkorp/opendlv-device-ps3controller-amd64:$VERSION -f Dockerfile.amd64 . &
docker build -t openkorp/opendlv-device-ps3controller-armhf:$VERSION -f Dockerfile.armhf .

docker push  openkorp/opendlv-device-ps3controller-aarch64:$VERSION
docker push  openkorp/opendlv-device-ps3controller-amd64:$VERSION 
docker push  openkorp/opendlv-device-ps3controller-armhf:$VERSION 

cat <<EOF >/tmp/multi.yml
image: openkorp/opendlv-device-ps3controller-multi:$VERSION
manifests:  
  - image: openkorp/opendlv-device-ps3controller-amd64:$VERSION
    platform:
      architecture: amd64
      os: linux
  - image: openkorp/opendlv-device-ps3controller-armhf:$VERSION
    platform:
      architecture: arm
      os: linux
  - image: openkorp/opendlv-device-ps3controller-aarch64:$VERSION
    platform:
      architecture: arm64
      os: linux
EOF
manifest-tool-linux-amd64 push from-spec /tmp/multi.yml