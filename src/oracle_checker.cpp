#include "fs_snapshot.h"

#ifdef MLFS
#include <mlfs/mlfs_interface.h>	
#endif

#define MINIMUM_ARGUMENTS 4
#define ORACLE_FILE_INDEX 1
#define FS_DIR_INDEX 2
#define OUTPUT_FILE_INDEX 3

const std::vector<std::string> getAllFilePaths(std::string);
void reportFailure(FSSnapshot&, FSSnapshot&, std::string);
int reset(std::string fsDir);

/* 
  Checks the current state of the file system against the given oracle file.
  Usage: ./oracle_checker ORACLE_FILE FS_DIR OUTPUT_FILE
    
  ORACLE_FILE is the path to the file system snapshot
  FS_DIR is the path where the file system is mounted
  OUTPUT_FILE is the path to write the report to if the check fails
*/
int main(int argc, char** argv) {    
    if (argc < MINIMUM_ARGUMENTS) {
        std::cout << "Specify ORACLE_FILE, FS_DIR and OUTPUT_FILE." << std::endl;
        return -1;
    }

    std::string oracleFile(argv[ORACLE_FILE_INDEX]);
    std::string fsDir(argv[FS_DIR_INDEX]);
    std::string outputFile(argv[OUTPUT_FILE_INDEX]);

    std::cout << "oracle_checker " << oracleFile << " " << fsDir << " " << outputFile << std::endl;

#ifdef MLFS
    init_fs();
#endif

    int result = -1;
    FSSnapshot oracleSnapshot(oracleFile);
    FSSnapshot currentSnapshot(fsDir, getAllFilePaths(fsDir));
    if (oracleSnapshot == currentSnapshot) {
        std::cout << "TEST PASSED" << std::endl;
        result = 0;
        // reportFailure(oracleSnapshot, currentSnapshot, outputFile);
    } else {
        std::cout << "TEST FAILED" << std::endl;
        reportFailure(oracleSnapshot, currentSnapshot, outputFile);
    }

    result |= reset(fsDir);

#ifdef MLFS
    shutdown_fs();
#endif

    return result;
}

const std::vector<std::string> getAllFilePaths(std::string fsDir) {
    std::vector<std::string> paths;
    paths.push_back(fsDir + "/A/C/oof");
    paths.push_back(fsDir + "/A/C/bar");
    paths.push_back(fsDir + "/A/C");

    paths.push_back(fsDir + "/A/oof");
    paths.push_back(fsDir + "/A/foo");
    paths.push_back(fsDir + "/A/bar");
    paths.push_back(fsDir + "/A");

    paths.push_back(fsDir + "/D/oof");
    paths.push_back(fsDir + "/D/bar");
    paths.push_back(fsDir + "/D");

    paths.push_back(fsDir + "/B/foo");
    paths.push_back(fsDir + "/B/bar");
    paths.push_back(fsDir + "/B");

    paths.push_back(fsDir + "/oof");
    paths.push_back(fsDir + "/bar");
    paths.push_back(fsDir + "/foo");
    return paths;
}

void reportFailure(FSSnapshot& oracleSnapshot, FSSnapshot& currentSnapshot, std::string outputFile) {
    std::ofstream out;
    out.open(outputFile, std::ios::out|std::ios::trunc);
    if ( ! out.is_open()) {
        std::cout << "Failed to open report file" << std::endl;
        return;
    }
    out << "ORACLE SNAPSHOT\n";
    oracleSnapshot.printState(out);
    out << "\nCURRENT SNAPSHOT\n";
    currentSnapshot.printState(out);
    out.close(); 
}

void reset(std::string fsDir) {
    int result = 0;
    std::cout << "Resetting crash directory" << std::endl;
    std::vector<std::string> filePaths = getAllFilePaths(fsDir);
    for (std::string file : filePaths) {
	    int err = remove(file.c_str());
        if (err < 0 && err != -ENOENT && errno != ENOENT) {
            perror("*** Failed to remove in reset");
            result = -1;
        }
    }
    return result;
}
