#ifdef MLFS
#include <mlfs/mlfs_interface.h>	
#endif

// C filesys operations
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>

void printStats(struct stat*);

int main() {
	#ifdef MLFS
		init_fs();
	#endif

	mkdir("/mlfs/test5", 0777);
	

#ifdef MLFS
	shutdown_fs();
#endif

	return 0;
}

void printStats(struct stat* snapshot) {
	std::cout << "\t\tDevice: " << snapshot->st_dev << std::endl;
	std::cout << "\t\tInode: " << snapshot->st_ino << std::endl;
	std::cout << "\t\tMode: " << snapshot->st_mode << std::endl;
	std::cout << "\t\tHard Links: " << snapshot->st_nlink << std::endl;
	std::cout << "\t\tUser ID: " << snapshot->st_uid << std::endl;
	std::cout << "\t\tGroup ID: " << snapshot->st_gid << std::endl;
	std::cout << "\t\tRdev: " << snapshot->st_rdev << std::endl;
	std::cout << "\t\tSize: " << snapshot->st_size << std::endl;
	std::cout << "\t\tBlock size: " << snapshot->st_blksize << std::endl;
	std::cout << "\t\tBlocks: " << snapshot->st_blocks << std::endl;
	std::cout << "\t\tAccess time: " << (long long)snapshot->st_atim.tv_sec << "." << snapshot->st_atim.tv_nsec << std::endl;
	std::cout << "\t\tModified time: " << (long long)snapshot->st_mtim.tv_sec << "." << snapshot->st_mtim.tv_nsec << std::endl;
	std::cout << "\t\tChange time: " << (long long)snapshot->st_ctim.tv_sec << "." << snapshot->st_ctim.tv_nsec << std::endl;
}

