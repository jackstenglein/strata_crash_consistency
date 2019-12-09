#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <iostream>

#define ORACLE_EXE_PATH "build/strata_read_ace_workload"
#define CRASH_EXE_PATH "build/strata_read_ace_workload"
#define CHECKER_EXE_PATH "build/oracle_checker"

#define WORKLOAD_DIR_INDEX 1
#define ORACLE_DIR_INDEX 2
#define REPORT_DIR_INDEX 3

void runTest(std::string, std::string, std::string, std::string);
int runOracle(const char*, const char*);
int runCrasher(const char*);
int runChecker(const char*, const char*);

/*
	Usage: ./test_harness WORKLOAD_DIR ORACLE_DIR REPORT_DIR
*/ 
int main(int argc, char** argv) {

	if (argc < 4) {
		std::cout << "Usage: ./test_harness WORKLOAD_DIR ORACLE_DIR REPORT_DIR" << std::endl;
	}

	std::string workloadDir(argv[WORKLOAD_DIR_INDEX]);
	std::string oracleDir(argv[ORACLE_DIR_INDEX]);
	std::string reportDir(argv[REPORT_DIR_INDEX]);

	// Read all files in workload directory
	struct dirent *entry = nullptr;
    DIR *dp = nullptr;
    dp = opendir(workloadDir.c_str());
    if (dp != nullptr) {
        while ((entry = readdir(dp))) {
			std::string workloadName(entry->d_name);
			if (workloadName != "." && workloadName != "..") {
				runTest(workloadDir, oracleDir, reportDir, workloadName);
			}
		}
    }

    closedir(dp);
}

void runTest(std::string workloadDir, std::string oracleDir, std::string reportDir, std::string workloadName) {
	std::cout << "Testing " << workloadName << std::endl;

	std::string separator("/");
	std::string workloadFile = (workloadDir + separator + workloadName);
	std::string oracleFile = (oracleDir + separator + workloadName);
	std::string reportFile = (reportDir + separator + workloadName);
	int status;

	std::cout << "Workload file: " << workloadFile << std::endl;
	std::cout << "Oracle file: " << oracleFile << std::endl;
	std::cout << "Report file: " << reportFile << std::endl;

	status = runOracle(workloadFile.c_str(), oracleFile.c_str());
	if (status < 0) {
		std::cout << "Oracle Failed!" << std::endl;
		return;
	}

	status = runCrasher(workloadFile.c_str());
	if (status < 0) {
		std::cout << "Crasher failed!" << std::endl;
		return;
	}
	
	status = runChecker(oracleFile.c_str(), reportFile.c_str());
	if (status < 0) {
		std::cout << "Checker failed!" << std::endl;
		return;
	}
}

int runOracle(const char* workloadFile, const char* oracleFile) {
	pid_t cpid = fork();
	if (cpid == -1) {
		perror("Failed to fork oracle");
		return -1;
	}
	if (cpid == 0) {
		// Child process
		execl(ORACLE_EXE_PATH, "strata_read_ace_workload", workloadFile, "test/oracle", "oracle", oracleFile, NULL);
		perror("Failed to exec oracle");
		exit(EXIT_FAILURE);
	} 

	int status;
	pid_t wpid = waitpid(cpid, &status, 0);
	if (wpid == -1) {
		perror("Failed to wait on oracle");
		return -1;
	}
	std::cout << "Finished waiting on oracle" << std::endl;
	return status;
}

int runCrasher(const char* workloadFile) {
	pid_t cpid = fork();
	if (cpid == -1) {
		perror("Failed to fork crasher");
		return -1;
	}
	if (cpid == 0) {
		execl(CRASH_EXE_PATH, "strata_read_ace_workload", workloadFile, "test/crash", "crash", NULL);
		perror("Failed to exec oracle");
		exit(EXIT_FAILURE);
	}

	std::cout << "WAITING ON CRASHER" << std::endl;
	int status = 0;
	pid_t wpid = wait(NULL);
	if (wpid == -1) {
		perror("Failed to wait on crasher");
		return -1;
	}
	std::cout << "Finished waiting on crasher" << std::endl;
	return status;
}

int runChecker(const char* oracleFile, const char* reportFile) {
	pid_t cpid = fork();
	if (cpid == -1) {
		perror("Failed to fork checker");
		return -1;
	}
	if (cpid == 0) {
		execl(CHECKER_EXE_PATH, "oracle_checker", oracleFile, "test/crash", reportFile, NULL);
		perror("Failed to exec checker");
		exit(EXIT_FAILURE);
	}

	int status;
	pid_t wpid = waitpid(cpid, &status, 0);
	if (wpid == -1) {
		perror("Failed to wait on crasher");
		return -1;
	}
	std::cout << "Finished waiting on checker" << std::endl;
	return status;
}
