#include "ace_runner.h"
#include "fs_snapshot.h"
#include <string>
#include <iostream>
#include <sys/types.h> /* for pid_t */
#include <signal.h>  /* for kill */
#include <errno.h>

// C filesys operations
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

AbstractAceRunner::AbstractAceRunner(void) {
	actionHandlers.emplace("checkpoint", &AbstractAceRunner::handle_checkpoint);
	actionHandlers.emplace("close", &AbstractAceRunner::handle_close);
	actionHandlers.emplace("falloc", &AbstractAceRunner::handle_falloc);
	actionHandlers.emplace("fdatasync", &AbstractAceRunner::handle_fdatasync);
	actionHandlers.emplace("fsync", &AbstractAceRunner::handle_fsync);
	actionHandlers.emplace("mkdir", &AbstractAceRunner::handle_mkdir);
	actionHandlers.emplace("open", &AbstractAceRunner::handle_open);
	actionHandlers.emplace("opendir", &AbstractAceRunner::handle_opendir);
	actionHandlers.emplace("remove", &AbstractAceRunner::handle_remove);
	actionHandlers.emplace("rename", &AbstractAceRunner::handle_rename);
	actionHandlers.emplace("sync", &AbstractAceRunner::handle_sync);
	actionHandlers.emplace("truncate", &AbstractAceRunner::handle_truncate);
	actionHandlers.emplace("unlink", &AbstractAceRunner::handle_unlink);
	actionHandlers.emplace("write", &AbstractAceRunner::handle_write);
}


const std::set<std::string> getFilePaths(std::string prefix) {
	std::set<std::string> paths;
	paths.insert(prefix + "/A/C/foo");
	paths.insert(prefix + "/A/C/bar");
	paths.insert(prefix + "/A/foo");
	paths.insert(prefix + "/B/foo");
	paths.insert(prefix + "/A/bar");
	paths.insert(prefix + "/B/bar");
	paths.insert(prefix + "/foo");
	paths.insert(prefix + "/bar");
	paths.insert(prefix + "/A/C");
	paths.insert(prefix + "/A");
	paths.insert(prefix + "/B");
	return paths;
}

std::string get_path(std::string file) {
    // Ace only has a finite number of paths, so just check for each individually
    std::string path("test");
    if (file == "ACfoo") return path + "/A/C/foo";
    if (file == "ACbar") return path + "/A/C/bar";
    if (file == "Afoo") return (path + "/A/foo");
    if (file == "Bfoo") return (path + "/B/foo");
    if (file == "Abar") return (path + "/A/bar");
    if (file == "Bbar") return (path + "/B/bar");
    if (file == "foo") return (path + "/foo");
    if (file == "bar") return (path + "/bar");
    if (file == "A") return (path + "/A");
    if (file == "AC") return (path + "/A/C");
    if (file == "B") return (path + "/B");
    if (file == "test") return path;
    return nullptr;
}

int AbstractAceRunner::handle_action(std::vector<std::string>& tokens) {
	std::string action = tokens[0];
	auto it = actionHandlers.find(action);
	if (it == actionHandlers.end()) {
		std::cout << "Could not find action " << action << std::endl;
		return -1;
	}
	return (this->*it->second)(tokens);
}

int AbstractAceRunner::handle_close(std::vector<std::string>& tokens) {
	std::cout << "close" << std::endl;
	int fd = fileDescriptors[get_path(tokens[1])];            
	return close(fd);
}

int AbstractAceRunner::handle_falloc(std::vector<std::string>& tokens) {
	const int filenameIndex = 1;
	const int flagIndex = 2;
    const int offset_index = 3;
    const int length_index = 4;

    const int fd = fileDescriptors[get_path(tokens[filenameIndex])];
    const int flags = parse_falloc_flags(tokens[flagIndex]);
    const int offset = std::stoi(tokens[offset_index], nullptr, 10);
    const int length = std::stoi(tokens[length_index], nullptr, 10);

    return fallocate(fd, flags, offset, length);    
}

int AbstractAceRunner::handle_fdatasync(std::vector<std::string>& tokens) {
	std::cout << "fdatasync" << std::endl;
	return fdatasync(fileDescriptors[get_path(tokens[1])]);
}

int AbstractAceRunner::handle_fsync(std::vector<std::string>& tokens) {
	std::cout << "fsync" << std::endl;
	if(tokens[1] == "test") {
		return fsync(test_fd);
	}
	else {
		return fsync(fileDescriptors[get_path(tokens[1])]);
	}
}

int AbstractAceRunner::handle_mkdir(std::vector<std::string>& tokens) {
	const int dir_index = 1;
    const int perm_index = 2;
    std::string c_dir_name = get_path(tokens[dir_index]);
    int permissions = std::stoi(tokens[perm_index], nullptr, 8);
    std::cout << "Create directory " << c_dir_name << " with base 10 permissions " << permissions << std::endl;
    return mkdir(c_dir_name.c_str(), (mode_t)permissions);
}

int AbstractAceRunner::handle_open(std::vector<std::string>& tokens) {
    const int file_index = 1;
	const int flag_index = 2;
    const int perm_index = 3;

    std::string file_path = get_path(tokens[file_index]);
    const int flags = parse_open_flags(tokens[flag_index]);
    const int permissions = std::stoi(tokens[perm_index], nullptr, 8);

    std::cout << "Flags string: " << tokens[flag_index] << std::endl;
    std::cout << "Open file " << file_path << " with base 10 permissions " << permissions << " and base 10 flags " << flags << std::endl;
    
    int fd = open(file_path.c_str(), flags, permissions);

    if (fd > 0) {
        fileDescriptors[file_path] = fd;
        if(tokens[1] == "test") {
            test_fd = fd;
        }
    }
    return fd;
}

int AbstractAceRunner::handle_opendir(std::vector<std::string>& tokens) {
	std::cout << "opendir" << std::endl;
	tokens.insert(tokens.begin() + 2, "O_DIRECTORY");
	int fd = handle_open(tokens);
	if (fd < 0) {
		std::cout << "Failed to open directory." << std::endl;
	}
	return fd;
}

int AbstractAceRunner::handle_remove(std::vector<std::string>& tokens) {
	const int filenameIndex = 1;
	return remove(get_path(tokens[filenameIndex]).c_str());
}

int AbstractAceRunner::handle_rename(std::vector<std::string>& tokens) {
	std::cout << "Renaming file " << std::endl;
    return rename( get_path(tokens[1]).c_str(), get_path(tokens[2]).c_str() );
}

int AbstractAceRunner::handle_sync(std::vector<std::string>& tokens) {
	std::cout << "sync" << std::endl;
	sync();
	return 1;
}

int AbstractAceRunner::handle_truncate(std::vector<std::string>& tokens) {
	std::string file_path = get_path(tokens[1]);
    int length = stoi(tokens[2]);
    return truncate(file_path.c_str(), length);
}

int AbstractAceRunner::handle_unlink(std::vector<std::string>& tokens) {
	std::cout << "Unlinking (deleting) file " << std::endl; 
	std::string path = get_path(tokens[1]);
	return unlink(path.c_str()); 
}

int AbstractAceRunner::handle_write(std::vector<std::string>& tokens) {
	const int perm_index = 3;
    std::string file_path = get_path(tokens[1]);
    int fd = fileDescriptors[file_path];
    int count = stoi(tokens[3]);
    std::string s(count, '0');
    return write(fd, s.c_str(), count);
}

int AbstractAceRunner::parse_falloc_flags(std::string flags) {
    if (flags == "0") {
        return 0;
    }

    int result = 0;
    if (flags.find("FALLOC_FL_ZERO_RANGE") != std::string::npos) {
        result |= FALLOC_FL_ZERO_RANGE;
    }
    if (flags.find("FALLOC_FL_KEEP_SIZE") != std::string::npos) {
        result |= FALLOC_FL_KEEP_SIZE;
    }
    if (flags.find("FALLOC_FL_PUNCH_HOLE") != std::string::npos) {
        result |= FALLOC_FL_PUNCH_HOLE;
    }
    return result;
}

int AbstractAceRunner::parse_open_flags(std::string flags) {
    int result = 0;
    // Check access modes
    if (flags.find("O_RDONLY") != std::string::npos) {
        result |= O_RDONLY;
    }
    if (flags.find("O_WRONLY") != std::string::npos) {
        result |= O_WRONLY;
    }
    if (flags.find("O_RDWR") != std::string::npos) {
        result |= O_RDWR;
    }

    // Check other flags
    if (flags.find("O_CREAT") != std::string::npos) {
        result |= O_CREAT;
    }
    if (flags.find("O_DIRECTORY") != std::string::npos) {
        result |= O_DIRECTORY;
    }

    return result;
}

int OracleAceRunner::handle_checkpoint(std::vector<std::string>& tokens) {
	std::cout << "Checkpoint, saving file state." << std::endl;
	// Crash happens
	FSSnapshot snapshot(getFilePaths("/mlfs"));
	snapshot.printState();
	snapshot.writeToFile("testfile.txt");
	return 1;
}

//gets strata's pid to kill it
pid_t get_pid() {
    char buf[512];
    FILE *cmd_pipe = popen("pidof -s kernfs", "r");
    fgets(buf, 512, cmd_pipe);
    pid_t pid = strtoul(buf, NULL, 10);
    pclose(cmd_pipe);  
    return pid;
}

int CrashAceRunner::handle_checkpoint(std::vector<std::string>& tokens) {
	std::cout << "Checkpoint, crashing from CrashAceRunner." << std::endl;
    pid_t pid = get_pid();
    //kill strata
    errno = kill(pid, SIGKILL);
    if(!errno) {
        std::cout << "Failed to kill kernfs (strata)." << std::endl;
    }
    //kill ourselves
    raise(SIGKILL);
	return 1;
}
