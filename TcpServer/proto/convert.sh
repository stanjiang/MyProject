#! /bin/bash

protoc --cpp_out=./include/ ./cs_proto/*.proto
rename .cc .cpp ./include/cs_proto/*
