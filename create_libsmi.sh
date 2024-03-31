#!/bin/bash -xe

# Delete old library if exists
rm -f libsmi.a

# Compile
make build -j

# Create library
arm-none-eabi-ar -vq libsmi.a @build/APP_CY8CPROTO-062-4343W/Debug/objlist.rsp
