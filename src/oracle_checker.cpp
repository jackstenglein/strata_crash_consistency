#include "fs_snapshot.h"

#ifdef MLFS
#include <mlfs/mlfs_interface.h>	
#endif

#define MINIMUM_ARGUMENTS 4
#define ORACLE_FILE_INDEX 1
#define FS_DIR_INDEX 2
#define OUTPUT_FILE_INDEX 3

const std::set<std::string> getAllFilePaths(std::string);
void reportFailure(FSSnapshot&, FSSnapshot&, std::string);

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

#ifdef MLFS
	init_fs();
#endif

	FSSnapshot oracleSnapshot(oracleFile);
	FSSnapshot currentSnapshot(getAllFilePaths(fsDir));
	if (oracleSnapshot == currentSnapshot) {
		std::cout << "TEST PASSED" << std::endl;
		reportFailure(oracleSnapshot, currentSnapshot, outputFile);
	} else {
		std::cout << "TEST FAILED" << std::endl;
		reportFailure(oracleSnapshot, currentSnapshot, outputFile);
	}

#ifdef MLFS
    shutdown_fs();
#endif

	return 0;
}

const std::set<std::string> getAllFilePaths(std::string fsDir) {
	std::set<std::string> paths;
	paths.insert(fsDir + "/A/C/foo");
	paths.insert(fsDir + "/A/C/bar");
	paths.insert(fsDir + "/A/foo");
	paths.insert(fsDir + "/B/foo");
	paths.insert(fsDir + "/A/bar");
	paths.insert(fsDir + "/B/bar");
	paths.insert(fsDir + "/foo");
	paths.insert(fsDir + "/bar");
	paths.insert(fsDir + "/A/C");
	paths.insert(fsDir + "/A");
	paths.insert(fsDir + "/B");
	return paths;
}

void reportFailure(FSSnapshot& oracleSnapshot, FSSnapshot& currentSnapshot, std::string outputFile) {
	std::ofstream out;
	out.open(outputFile, std::ios::out|std::ios::trunc);
	if ( ! out.is_open()) {
		std::cout << "Failed to open output file" << std::endl;
		return;
	}
	out << "ORACLE SNAPSHOT\n";
	oracleSnapshot.printState(out);
	out << "\nCURRENT SNAPSHOT\n";
	currentSnapshot.printState(out);
	out.close(); 
}
