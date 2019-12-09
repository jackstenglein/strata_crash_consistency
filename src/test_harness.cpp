#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>

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
            std::cout << "Testing " << workloadName << std::endl;
		}
    }

    closedir(dp);
}

void runTest(std::string workloadDir, std::string oracleDir, std::string reportDir, std::string workloadName) {

	std::string separator("/");
	const char* workloadFile = (workloadDir + separator + workloadName).c_str();
	const char* oracleFile = (oracleDir + separator + workloadName).c_str();
	const char* reportFile = (reportDir + separator + workloadName).c_str();
	int status;

	status = runOracle(workloadFile, oracleFile);
	if (status < 0) {
		std::cout << "Oracle Failed!" << std::endl;
		return;
	}

	status = runCrasher(workloadFile);
	if (status < 0) {
		std::cout << "Crasher failed!" << std::endl;
		return;
	}
	
	status = runChecker(workloadFile, reportFile);
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

	int status
	pid_t wpid = waitpid(cpid, &status, 0);
	if (wpid == -1) {
		perror("Failed to wait on crasher");
		return -1;
	}
	return status;
}

int runChecker(const char* workloadFile, const char* reportFile) {
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
	return status;
}
