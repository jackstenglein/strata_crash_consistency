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

// using namespace std;

#define FILENAME_INDEX 1
#define FLAGS_INDEX 2
#define WORKLOAD_DIR "test"


std::vector<std::string> tokenize(std::string str);
void run_test(std::string test_file);
void reset();
const std::set<std::string> getFilePaths(std::string prefix);
int test_fd;

// #ifdef MLFS
// const char test_dir_prefix[] = "/mlfs";
// #else
const char test_dir_prefix[] = "./test";
// #endif

std::set<std::string> paths;
std::map<std::string, int> paths_to_fds;
//std::set<std::string> paths_added;

int main(int argc, char** argv)
{    

// #ifdef MLFS
// 	init_fs();
// #endif

    //std::set<std::string> unhandled_actions;

    std::string workload_dir = std::string(WORKLOAD_DIR);
    std::string separator = "/";
    DIR *dir;
    //struct dirent *ent;
    if ((dir = opendir(WORKLOAD_DIR)) != NULL) {
        /* print all the files and directories within directory */
        //std::string file = std::string(ent->d_name);
        //std::cout << workload_dir + separator + file << std::endl;
        run_test(argv[1]);
        closedir(dir);
    } else {
        perror("Could not open directory test");
        return EXIT_FAILURE;
    }

// #ifdef MLFS
// 	shutdown_fs();
// #endif

	return 0;
}

void run_test(std::string test_file) {

    int err;
	std::string line;
	std::ifstream infile(test_file);
    while (std::getline(infile, line)) {
        if (line == "# run") break;
    }
    
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens = tokenize(line);
        
	CrashAceRunner runner; 
	
        runner.handle_action(tokens);
    }
    reset();
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


void reset() {
    std::cout << "RESET CALLED" << std::endl;
    //std::cout << paths_added.size() << std::endl;
#ifdef MLFS
    //remove files first
    remove("/mlfs/A/C/foo");
    remove("/mlfs/A/C/bar");
    remove("/mlfs/A/foo");
    remove("/mlfs/B/foo");
    remove("/mlfs/A/bar");
    remove("/mlfs/B/bar");
    remove("/mlfs/foo");
    remove("/mlfs/bar");
    remove("/mlfs/B/bar");
    
    //remove directories
    remove("/mlfs/A");
    remove("/mlfs/A/C");
    remove("/mlfs/B");
#else
    //remove files first
    remove("test/A/C/foo");
    remove("test/A/C/bar");
    remove("test/A/foo");
    remove("test/B/foo");
    remove("test/A/bar");
    remove("test/B/bar");
    remove("test/foo");
    remove("test/bar");
    remove("test/B/bar");
    
    //remove directories
    remove("test/A");
    remove("test/A/C");
    remove("test/B");
    
#endif
    //paths_added.clear();
}

