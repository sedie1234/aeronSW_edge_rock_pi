#!/bin/bash
export LD_LIBRARY_PATH=$(pwd)/third_party/librdkafka
valgrind --leak-check=yes ./bin/run
