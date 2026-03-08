#!/bin/bash

set -xe
        
gcc -Wall -Wextra -Iexternal ./src/main.c -o ./build/program -lm
