#!/bin/bash

WORKLOADS_DIR=j-lang-files/*
TEST_DIR=test
MLFS_DIR=/mlfs  #strata's mount location
ORACLE_FILE=oracle

for d in $WORKLOADS_DIR
do
    #get seq directories
    echo "Current directory is $d"
    SEQ_DIR="$d/*"
    #get all workload files in seq dir
    for f in $SEQ_DIR
    do
	#echo "seq dir = $SEQ_DIR"
	#echo "workload file = $f"
	#echo "test dir = $TEST_DIR"
	#echo "oracle file = $ORACLE_FILE"

	echo "testing workload $f"
	#run in oracle mode first
	./build/strata_read_ace_workload $f $MLFS_DIR oracle $ORACLE_FILE
	#run in crash mode
	./build/strata_read_ace_workload $f $MLFS_DIR crash
    done
done
