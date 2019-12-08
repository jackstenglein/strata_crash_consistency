#!/bin/bash

WORKLOADS_DIR=j-lang-files/*
TEST_DIR=test
MLFS_DIR=/mlfs  #strata's mount location
ORACLE_FILE=oracle/test.txt
OUTPUT_FILE=report/test.txt #write failed crash checking reports

#for d in $WORKLOADS_DIR
#do
    #get seq directories
    echo "Current directory is $d"
    SEQ_DIR="$d/*"
    #get all workload files in seq dir
    #for f in $SEQ_DIR
    #do
	#echo "seq dir = $SEQ_DIR"
	#echo "workload file = $f"
	#echo "test dir = $TEST_DIR"
	#echo "oracle file = $ORACLE_FILE"

	f=j-lang-files/seq1/j-lang101
	echo "testing workload $f"
	#run in oracle mode first
	./build/strata_read_ace_workload $f $MLFS_DIR oracle $ORACLE_FILE
	#run in crash mode
	./build/strata_read_ace_workload $f $MLFS_DIR crash
	#restart strata
	#sudo /home/strata_cc/strata/kernfs/tests/run.sh kernfs
	
	#run oracle checker
        #./build/oracle_checker $ORACLE_FILE $MLFS_DIR $OUTPUT_FILE
	
    #done
#done
