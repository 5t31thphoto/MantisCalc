#!/bin/sh
# Host-side logic tests for src/main.cpp (no hardware needed)
cd "$(dirname "$0")" && g++ -std=gnu++17 -Wno-unused-function -Imock test_main.cpp -o /tmp/mantis_t && /tmp/mantis_t
