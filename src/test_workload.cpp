#ifdef MLFS
#include <mlfs/mlfs_interface.h>	
#endif

// C filesys operations
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
	mkdir("/mlfs/test", 0777);
	int fd = open("/mlfs/test/foo", O_RDWR|O_CREAT, 0777);
	if (fd < 0) {
		perror("Failed to open");
	}
	fsync(fd);
	close(fd);
	return 0;
}