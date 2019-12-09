CC = g++
RUNNER_EXE = strata_read_ace_workload
CHECKER_EXE = oracle_checker
SOURCE_DIR = src
BUILD_DIR = build
RUNNER_SOURCES = $(SOURCE_DIR)/ace_runner.cpp $(SOURCE_DIR)/fs_snapshot.cpp $(SOURCE_DIR)/strata_read_ace_workload.cpp
RUNNER_OBJECTS = $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(RUNNER_SOURCES))
CHECKER_SOURCES = $(SOURCE_DIR)/fs_snapshot.cpp $(SOURCE_DIR)/oracle_checker.cpp
CHECKER_OBJECTS = $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CHECKER_SOURCES))

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

all: dir runner checker harness

runner: $(BUILD_DIR)/$(RUNNER_EXE)

checker: $(BUILD_DIR)/$(CHECKER_EXE)

harness: $(SOURCE_DIR)/test_harness.cpp
	$(CXX) -std=c++11 -o $(BUILD_DIR)/test_harness $^

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o : $(SOURCE_DIR)/%.cpp
	$(CXX) -g -c $< -o $@ -I$(INCLUDES) -L$(LIBFS_DIR) -lmlfs -DMLFS $(CFLAGS) $(LDFLAGS)

$(BUILD_DIR)/$(RUNNER_EXE): $(RUNNER_OBJECTS)
	$(CXX) -std=c++11 -g -Ofast -o $@ $^ -I$(INCLUDES) -L$(LIBFS_DIR) -lmlfs -DMLFS $(CFLAGS) $(LDFLAGS)

$(BUILD_DIR)/$(CHECKER_EXE): $(CHECKER_OBJECTS) 
	$(CXX) -std=c++11 -g -Ofast -o $@ $^ -I$(INCLUDES) -L$(LIBFS_DIR) -lmlfs -DMLFS $(CFLAGS) $(LDFLAGS)

testFile: src/test_workload.cpp
	$(CXX) -std=c++11 -g -Ofast -o $@ $^ -I$(INCLUDES) -L$(LIBFS_DIR) -lmlfs -DMLFS $(CFLAGS) $(LDFLAGS)

testOof: src/read_foo_fstats.cpp
	$(CXX) -std=c++11 -g -Ofast -o $@ $^ -I$(INCLUDES) -L$(LIBFS_DIR) -lmlfs -DMLFS $(CFLAGS) $(LDFLAGS)

testMkdir: src/mkdir_workload.cpp
	$(CXX) -std=c++11 -g -Ofast -o $@ $^ -I$(INCLUDES) -L$(LIBFS_DIR) -lmlfs -DMLFS $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)/*o $(BUILD_DIR)/$(RUNNER_EXE)
