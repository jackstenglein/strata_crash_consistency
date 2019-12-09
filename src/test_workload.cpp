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

#ifdef MLFS
	shutdown_fs();
#endif
	return 0;
}

void printState(std::ostream& out) const {
	out << "\t\tDevice: " << snapshot.st_dev << std::endl;
	out << "\t\tInode: " << snapshot.st_ino << std::endl;
	out << "\t\tMode: " << snapshot.st_mode << std::endl;
	out << "\t\tHard Links: " << snapshot.st_nlink << std::endl;
	out << "\t\tUser ID: " << snapshot.st_uid << std::endl;
	out << "\t\tGroup ID: " << snapshot.st_gid << std::endl;
	out << "\t\tRdev: " << snapshot.st_rdev << std::endl;
	out << "\t\tSize: " << snapshot.st_size << std::endl;
	out << "\t\tBlock size: " << snapshot.st_blksize << std::endl;
	out << "\t\tBlocks: " << snapshot.st_blocks << std::endl;
	out << "\t\tAccess time: " << (long long)snapshot.st_atim.tv_sec << "." << snapshot.st_atim.tv_nsec << std::endl;
	out << "\t\tModified time: " << (long long)snapshot.st_mtim.tv_sec << "." << snapshot.st_mtim.tv_nsec << std::endl;
	out << "\t\tChange time: " << (long long)snapshot.st_ctim.tv_sec << "." << snapshot.st_ctim.tv_nsec << std::endl;
}

