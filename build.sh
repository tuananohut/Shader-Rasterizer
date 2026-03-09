#!/bin/bash

set -xe
        
gcc -Iexternal ./src/main.c -o ./build/program -lm
