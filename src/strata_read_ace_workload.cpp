#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <dirent.h>
#include <set>
#include "fs_snapshot.h"
#include "ace_runner.h"

#ifdef MLFS
#include <mlfs/mlfs_interface.h>	
#endif

// C filesys operations
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#define MINIMUM_ARGUMENTS 5
#define ORACLE_TEST_MODE "oracle"
#define CRASH_TEST_MODE "crash"

#define WORKLOAD_PATH_INDEX 1
#define TEST_DIR_INDEX 2
#define TEST_MODE_INDEX 3
#define FINAL_ARG_INDEX 4

std::vector<std::string> tokenize(std::string str);
int runWorkload(std::string workloadFile, AbstractAceRunner* runner);
void reset();
const std::set<std::string> getFilePaths(std::string prefix);


/* 
  Runs an ACE workload with the given options.
  Usage: ./strata_read_ace_workload WORKLOAD_PATH TEST_DIR oracle|crash ORACLE_FILE|true|false
    
  WORKLOAD_PATH is the path to the ACE workload file
  TEST_DIR is the directory to run the ACE workload in 
  oracle specifies running with an OracleAceRunner object
  crash specifies running with a CrashAceRunner object
  ORACLE_FILE is where the file system snapshot should be saved if running in oracle mode.
  Specify true/false for crash mode, to indicate whether the file system should be crashed.
*/
int main(int argc, char** argv) {    
    if (argc < MINIMUM_ARGUMENTS) {
        std::cout << "Usage: ./strata_read_ace_workload WORKLOAD_PATH TEST_DIR oracle|crash ORACLE_FILE|true|false" << std::endl;
        return -1;
    }

    std::string workloadFile(argv[WORKLOAD_PATH_INDEX]);
    std::string testDir(argv[TEST_DIR_INDEX]);
    std::string testMode(argv[TEST_MODE_INDEX]);
    std::string finalArg(argv[FINAL_ARG_INDEX]);
    AbstractAceRunner* runner = NULL;

    if (testMode == ORACLE_TEST_MODE) {
        runner = new OracleAceRunner(testDir, finalArg);
    } else if (testMode == CRASH_TEST_MODE) {
        runner = new CrashAceRunner(testDir, finalArg);
    } else {
        std::cout << "Specify either oracle or crash mode."  << std::endl;
        return -1;
    }

    std::cout << "strata_read_ace_workload " << workloadFile << " " << testDir << " " << testMode << " " << finalArg << std::endl; 

#ifdef MLFS
    std::cout << "Initializing strata" << std::endl;
    init_fs();
#endif

    int result = runWorkload(workloadFile, runner);
    free(runner);

#ifdef MLFS
    shutdown_fs();
#endif

	return result;
}

int runWorkload(std::string workloadFile, AbstractAceRunner* runner) {

    std::string line;
    std::ifstream infile(workloadFile);
    while (std::getline(infile, line)) {
        if (line == "# run") break;
    }

    int result = 0;
    while (std::getline(infile, line)) {
        std::cout << line << std::endl;
        std::istringstream iss(line);
        std::vector<std::string> tokens = tokenize(line);
        result = runner->handle_action(tokens);
        if (result < 0) {
            std::cout << "FAILED workload on line: " << line << std::endl;
            perror("");
            break;
        }
    }
    // runner->reset();
    return 0;
}

std::vector<std::string> tokenize(std::string str) {
    std::string token = " ";
    std::vector<std::string> result;
    while (str.size()) {
        int index = str.find(token);
        if (index != std::string::npos) {
            result.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
        } else {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}
