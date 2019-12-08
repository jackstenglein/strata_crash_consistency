#!/bin/bash

SEQ_DIR=j-lang-files/*
TEST_DIR=test
#MLFS_DIR=  #strata's mount location
ORACLE_FILE=oracle
for d in $SEQ_DIR
do
    #get seq directories
    echo "Current directory is $d"
    WORKLOAD_DIR="$d/*"
    #get all workload files in seq dir
    for f in $WORKLOAD_DIR
    do
	#echo "$f"
	#run in oracle mode first
	./build/strata_read_ace_workload "$WORKLOAD_DIR" "$TEST_DIR" oracle "$ORACLE_FILE"
	#run in crash mode
	./build/strata_read_ace_workload "$WORKLOAD_DIR" "$TEST_DIR" crash
    done
done
