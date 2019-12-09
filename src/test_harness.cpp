#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sys/stat.h>

#ifdef MLFS
#include <mlfs/mlfs_interface.h>	
#endif


#define ORACLE_EXE_PATH "build/strata_read_ace_workload"
#define CRASH_EXE_PATH "build/strata_read_ace_workload"
#define CHECKER_EXE_PATH "build/oracle_checker"

#define WORKLOAD_DIR_INDEX 1
#define ORACLE_DIR_INDEX 2
#define REPORT_DIR_INDEX 3

int runTest(std::string, std::string);
int createTestDirectories(std::string, std::string);
int runOracle(std::string, std::string, std::string);
int runCrasher(std::string, std::string);
int runChecker(std::string, std::string, std::string);

/*
	Usage: ./test_harness WORKLOAD_DIR

	Generate oracle files are written to ./oracle and report files are written to ./report
*/ 
int main(int argc, char** argv) {

	if (argc < 2) {
		std::cout << "Usage: ./test_harness WORKLOAD_DIR" << std::endl;
		return -1;
	}

	std::string workloadDir(argv[WORKLOAD_DIR_INDEX]);

	// Read all files in workload directory
	int err;
	struct dirent *entry = nullptr;
    DIR *dp = nullptr;
    dp = opendir(workloadDir.c_str());
    if (dp != nullptr) {
        while ((entry = readdir(dp))) {
			std::string workloadName(entry->d_name);
			if (workloadName != "." && workloadName != "..") {
				err = runTest(workloadDir, workloadName);
				if (err) {
					std::cout << "Aborting remaining tests" << std::endl;
					return -1;
				}
			}
		}
    }

    closedir(dp);
}

int runTest(std::string workloadDir, std::string workloadName) {
	std::cout << "Testing " << workloadName << std::endl;

	std::string separator("/");
	std::string oracleDir = ("test/oracle-" + workloadName);
	std::string crashDir = ("test/crash-" + workloadName);
	std::string workloadFile = (workloadDir + separator + workloadName);
	std::string oracleFile = ("oracle/" + workloadName);
	std::string reportFile = ("report/" + workloadName);

	int status = createTestDirectories(oracleDir, crashDir);
	if (status != 0) {
		return -1;
	}

	status = runOracle(oracleDir, workloadFile, oracleFile);
	if (status != 0) {
		std::cout << "Oracle Failed!" << std::endl;
		return -1;
	}

	status = runCrasher(crashDir, workloadFile);
	if (status != 0) {
		std::cout << "Crasher failed!" << std::endl;
		return -1;
	}
	
	status = runChecker(oracleFile, crashDir, reportFile);
	if (status != 0) {
		std::cout << "Checker failed!" << std::endl;
		return -1;
	}

	return 0;
}

int createTestDirectories(std::string oracleDir, std::string crashDir) {

#ifdef MLFS
	init_fs();
#endif

	std::cout << "Making oracle directory: " << oracleDir << std::endl;
	int err = mkdir(oracleDir.c_str(), 0777);
	if (err != 0) {
		perror("Failed to make oracleDir");
		return err;
	}

	std::cout << "Making crash directory: " << crashDir << std::endl;
	err = mkdir(crashDir.c_str(), 0777);
	if (err != 0) {
		perror("Failed to make crashDir");
		return err;
	}

#ifdef MLFS
	shutdown_fs();
#endif

	return 0;
}

int runOracle(std::string oracleDir, std::string workloadFile, std::string oracleFile) {
	std::cout << "Running oracle in directory " << oracleDir << " with workload " << workloadFile << " and oracleFile: " << oracleFile << std::endl;
	pid_t cpid = fork();
	if (cpid == -1) {
		perror("Failed to fork oracle");
		return -1;
	}
	if (cpid == 0) {
		// Child process
		execl(ORACLE_EXE_PATH, "strata_read_ace_workload", workloadFile.c_str(), oracleDir.c_str(), "oracle", oracleFile.c_str(), NULL);
		perror("Failed to exec oracle");
		exit(EXIT_FAILURE);
	} 

	std::cout << "Waiting on oracle" << std::endl;
	int status;
	pid_t wpid = waitpid(cpid, &status, 0);
	if (wpid == -1) {
		perror("Failed to wait on oracle");
		return -1;
	}

	std::cout << "Finished waiting for oracle; got status: " << WEXITSTATUS(status) << std::endl;
	return WEXITSTATUS(status);
}

int runCrasher(std::string crashDir, std::string workloadFile) {
	std::cout << "Running crasher in directory " << crashDir << " with workload " << workloadFile << std::endl;
	pid_t cpid = fork();
	if (cpid == -1) {
		perror("Failed to fork crasher");
		return -1;
	}
	if (cpid == 0) {
		execl(CRASH_EXE_PATH, "strata_read_ace_workload", workloadFile.c_str(), crashDir.c_str(), "crash", "false", NULL);
		perror("Failed to exec crasher");
		exit(EXIT_FAILURE);
	}

	std::cout << "Waiting on crasher" << std::endl;
	int status = 0;
	pid_t wpid = wait(NULL);
	if (wpid == -1) {
		perror("Failed to wait on crasher");
		return -1;
	}

	std::cout << "Finished waiting on crasher; got status " << WEXITSTATUS(status) << std::endl;
	return WEXITSTATUS(status);
}

int runChecker(std::string oracleFile, std::string crashDir, std::string reportFile) {
	std::cout << "Running checker in directory " << crashDir << " with oracleFile " << oracleFile << " and reportFile " << reportFile << std::endl;
	pid_t cpid = fork();
	if (cpid == -1) {
		perror("Failed to fork checker");
		return -1;
	}
	if (cpid == 0) {
		execl(CHECKER_EXE_PATH, "oracle_checker", oracleFile.c_str(), crashDir.c_str(), reportFile.c_str(), NULL);
		perror("Failed to exec checker");
		exit(EXIT_FAILURE);
	}

	std::cout << "Waiting on checker" << std::endl;
	int status;
	pid_t wpid = waitpid(cpid, &status, 0);
	if (wpid == -1) {
		perror("Failed to wait on checker");
		return -1;
	}

	std::cout << "Finished waiting on checker; got status " << WEXITSTATUS(status) << std::endl;
	return WEXITSTATUS(status);
}
