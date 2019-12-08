
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cstdio>
#include <unistd.h>

#define GIGABYTE 1024*1024*1024

int main() {
	int fd = open("/dev/dax0.0", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "cannot open dax device 0.0\n");
		return -1;
	}

	char* dax_addr = (char*)mmap(NULL, 4*GIGABYTE, PROT_READ | PROT_WRITE, MAP_SHARED| MAP_POPULATE, fd, 0);
	memset(dax_addr, 0, 4*GIGABYTE-(550 << 21));
	close(fd);

	fd = open("/dev/dax1.0", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "cannot open dax device 1.0\n");
		return -1;
	}
	dax_addr = (char*)mmap(NULL, 2*GIGABYTE, PROT_READ | PROT_WRITE, MAP_SHARED| MAP_POPULATE, fd, 0);
	memset(dax_addr, 0, 2*GIGABYTE-(550 << 21));
	close(fd);
}
