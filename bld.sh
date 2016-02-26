#!/bin/sh
# bld.sh - script to build the CHIP "gpiotest" program.

gcc -Wall -O3 -std=c99 -o gpiotime gpiotime.c
