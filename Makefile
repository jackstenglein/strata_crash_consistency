CC = g++
EXE = strata_read_ace_workload

CUR_DIR = $(shell pwd)
LIBFS_DIR = $(CUR_DIR)/../strata/libfs/build/
NVML_DIR = $(LIBFS_DIR)/../lib/nvml/src/nondebug/
CUCKOO_DIR = $(LIBFS_DIR)/../lib/cuckoofilter
LIBFS = $(LIBFS_DIR)/libmlfs.a

INCLUDES = $(LIBFS_DIR)/../src -I$(LIBFS_DIR)/../lib/nvml/src/include
DAX_OBJ = .$(LIBFS_DIR)/storage/storage_dax.o

DPDK_VER = 16.07.2
DPDK_DIR = $(abspath $(LIBFS_DIR)/../lib/dpdk-$(DPDK_VER)/x86_64-native-linuxapp-gcc)

CFLAGS = -I$(DPDK_DIR)/include -I$(INCLUDES)

LDFLAGS = -Wl,-rpath=$(abspath $(LIBFS_DIR)) \
		  -Wl,-rpath=$(abspath $(NVML_DIR)) \
		  -lpthread -lrt -lm -lssl -lcrypto

all: $(EXE)

strata_read_ace_workload: strata_read_ace_workload.o ace_runner.o fs_snapshot.o
	$(CXX) -std=c++11 -g -Ofast -o $@ $^ -I$(INCLUDES) -L$(LIBFS_DIR) -lmlfs -DMLFS $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o $(EXE) *.normal
