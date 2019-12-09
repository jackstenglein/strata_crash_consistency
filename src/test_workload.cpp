#ifdef MLFS
#include <mlfs/mlfs_interface.h>	
#endif

// C filesys operations
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

void printState(

int main() {
	#ifdef MLFS
		init_fs();
	#endif
	mkdir("/mlfs/test", 0777);
	int fd = open("/mlfs/test/foo", O_RDWR|O_CREAT, 0777);
	if (fd < 0) {
		perror("Failed to open");
	}
	fsync(fd);
	close(fd);
	struct stat result;
	stat("/mlfs/test/foo", &result);
	std::cout << "Stats before digestion: " << std::endl;
	printStats(&result);

#ifdef MLFS
	shutdown_fs();
	init_fs();
#endif

	stat("/mlfs/test/foo", &result);
	std::cout << "Stats after digestion: " << std::endl;
	printStats(&result);

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

