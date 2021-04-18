#!/usr/bin/bash

Xephyr -br -ac -noreset -screen 1720x900 :1 &
sleep 0.1
DISPLAY=:1 ./Entry

