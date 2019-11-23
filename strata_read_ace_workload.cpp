#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <dirent.h>
#include <set>

// C filesys operations
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

// using namespace std;

#define FILENAME_INDEX 1
#define FLAGS_INDEX 2
#define TEST_DIR "j-lang-files"


std::vector<std::string> tokenize(std::string str);
void run_test(std::string test_file, std::set<std::string> &unhandled_actions);
int handle_mkdir(std::vector<std::string> tokens, std::set<std::string> &paths_added);
int handle_open(std::vector<std::string> tokens, std::set<std::string> &paths_added);
int handle_truncate(std::vector<std::string> tokens);
int handle_write(std::vector<std::string> tokens);
const char* get_path(std::string file);
int parse_open_flags(std::string flags);
int handle_falloc(std::vector<std::string> tokens);
int parse_falloc_flags(std::string flags);
void reset(std::set<std::string> &paths_added);

std::map<std::string, int> paths_to_fds;
int main(int argc, char** argv)
{    std::set<std::string> unhandled_actions;

    std::string test_dir = std::string(TEST_DIR);
    std::string separator = "/";
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(TEST_DIR)) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            std::string file = std::string(ent->d_name);
            if (file != "." && file != "..") {
                std::cout << test_dir + separator + file << std::endl;
                run_test(test_dir + separator + file, unhandled_actions);
            }
        }
        closedir(dir);
    } else {
        perror("Could not open directory test");
        return EXIT_FAILURE;
    }

    std::cout << "Unhandled Actions: ";
    for (std::set<std::string>::iterator it=unhandled_actions.begin(); it!=unhandled_actions.end(); ++it)
        std::cout << ' ' << *it << std::endl;
	return 0;
}

void run_test(std::string test_file, std::set<std::string> &unhandled_actions) {

    int err;
	std::string line;
	std::ifstream infile(test_file);
    std::set<std::string> paths_added;
    while (std::getline(infile, line)) {
        if (line == "# run") break;
    }
    
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens = tokenize(line);
        std::string action = tokens[0];
        //std::cout << action << ' ' << std::endl;
        if (action == "checkpoint") {
            std::cout << "Checkpoint, need to crash." << std::endl;
            //Crash happens
        } else if (action == "sync") {
            std::cout << "sync" << std::endl;
            sync();
        } else if (action == "fsync") {
            std::cout << "fsync" << std::endl;
            //fsync in strata is a no-op, but call it anyway
            if(fsync(paths_to_fds[get_path(tokens[1])])) {
                std::cout << "Failed to fsync" << std::endl;
            }
        } else if (action == "fdatasync") {
            std::cout << "fdatasync" << std::endl;
            if(fdatasync(paths_to_fds[get_path(tokens[1])])) {
                std::cout << "Failed to fdatasync" << std::endl;
            }
        } else if (action == "mkdir") {
            if (handle_mkdir(tokens, paths_added)) {
                std::cout << "Failed to create directory." << std::endl;
            }
        } else if (action == "open") {
            int fd = handle_open(tokens, paths_added);
            if (fd < 0) {
                std::cout << "Failed to open file." << std::endl;
            }
        } else if (action == "opendir") {
            std::cout << "opendir" << std::endl;
            tokens.insert(tokens.begin() + 2, "O_DIRECTORY");
            int fd = handle_open(tokens, paths_added);
            if (fd < 0) {
                std::cout << "Failed to open directory." << std::endl;
            }
        } else if (action == "close") {
            std::cout << "close" << std::endl;
            int fd = paths_to_fds[get_path(tokens[1])];
            if (close(fd)) {
                std::cout << "Failed to close file." << std::endl;
            }
        } else if (action == "rename") {
            std::cout << "Renaming file " << std::endl;
            if (rename(get_path(tokens[1]), get_path(tokens[2]))) {
                std::cout << "Failed to rename " << tokens[1] << " to " << tokens[2] << std::endl;
            }
            paths_added.erase(get_path(tokens[1]));
            paths_added.insert(get_path(tokens[2]));
        } else if (action == "truncate") {
            std::cout << "Truncating file " << std::endl;
            int result = handle_truncate(tokens);
            if (result) {
                std::cout << "Failed to truncate file" << std::endl;
            }
        } else if (action == "unlink") {
            std::cout << "Unlinking (deleting) file " << std::endl; 
            const char* path = get_path(tokens[1]);
            int result = unlink(path);
            if(result) {
                std::cout << "Failed to unlink (delete) file" << std::endl;
            }             
        } else if (action == "write") {
            std::cout << "Writing file " << std::endl;
            if(!handle_write(tokens)) {
                std::cout << "Failed to write to file" << std::endl;
            }
        } else if (action == "falloc") { 
            if (handle_falloc(tokens)) {
                std::cout << "Failed to fallocate" << std::endl;
            }
        } else if (action == "remove") {
            if (remove(get_path(tokens[FILENAME_INDEX]))) {
                std::cout << "Failed to remove " << tokens[FILENAME_INDEX] << std::endl;
            }
            paths_added.erase(tokens[FILENAME_INDEX]);
        } else {
            unhandled_actions.insert(action);
        }
    }    
    reset(paths_added);
}

std::vector<std::string> tokenize(std::string str) {
    std::string token = " ";
    std::vector<std::string> result;
    while (str.size()) {
        int index = str.find(token);
        if (index != std::string::npos) {
            result.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
        } else {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

int handle_mkdir(std::vector<std::string> tokens, std::set<std::string> &paths_added) {
    const int dir_index = 1;
    const int perm_index = 2;
    const char* c_dir_name = tokens[dir_index].c_str();
    int permissions = std::stoi(tokens[perm_index], nullptr, 8);
    std::cout << "Create directory " << c_dir_name << " with base 10 permissions " << permissions << std::endl;
    paths_added.insert(c_dir_name);
    return mkdir(c_dir_name, (mode_t)permissions);
}

int handle_open(std::vector<std::string> tokens, std::set<std::string> &paths_added) {
    
    const int file_index = 1;
    const int perm_index = 3;

    const char* file_path = get_path(tokens[file_index]);
    const int flags = parse_open_flags(tokens[FLAGS_INDEX]);
    const int permissions = std::stoi(tokens[perm_index], nullptr, 8);

    std::cout << "Flags string: " << tokens[FLAGS_INDEX] << std::endl;
    std::cout << "Open file " << file_path << " with base 10 permissions " << permissions << " and base 10 flags " << flags << std::endl;
    
    int fd = open(file_path, flags, permissions);
    if (fd > 0) {
        paths_to_fds[tokens[file_index]] = fd;
        paths_added.insert(file_path);
    }
    return fd;
}

int handle_truncate(std::vector<std::string> tokens) {
    const char* file_path = get_path(tokens[1]);
    int length = stoi(tokens[2]);
    return truncate(file_path, length);
}

int handle_write(std::vector<std::string> tokens) {
    const int perm_index = 3;
    const char* file_path = get_path(tokens[1]);
    int fd = paths_to_fds[tokens[1]];
    int count = stoi(tokens[3]);
    std::string s(count, '0');
    return write(fd, s.c_str(), count);
}


const char* get_path(std::string file) {
    // Ace only has a finite number of paths, so just check for each individually
    if (file == "ACfoo") return "A/C/foo";
    if (file == "ACbar") return "A/C/bar";
    if (file == "Afoo") return "A/foo";
    if (file == "Bfoo") return "B/foo";
    if (file == "Abar") return "A/bar";
    if (file == "Bbar") return "B/bar";
    if (file == "foo") return "foo";
    if (file == "bar") return "bar";
    if (file == "test") return "test";
    if (file == "A") return "A";
    if (file == "AC") return "A/C";
    if (file == "B") return "B";
    return nullptr;
}

int parse_open_flags(std::string flags) {
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

int handle_falloc(std::vector<std::string> tokens) {
    const int offset_index = 3;
    const int length_index = 4;

    const int fd = paths_to_fds[get_path(tokens[FILENAME_INDEX])];
    const int flags = parse_falloc_flags(tokens[FLAGS_INDEX]);
    const int offset = std::stoi(tokens[offset_index], nullptr, 10);
    const int length = std::stoi(tokens[length_index], nullptr, 10);

    return fallocate(fd, flags, offset, length);    
}

int parse_falloc_flags(std::string flags) {
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

void reset(std::set<std::string> &paths_added) {
    std::cout << "RESET CALLED" << std::endl;
    std::cout << paths_added.size() << std::endl;
    for(std::set<std::string>::iterator it = paths_added.begin(); it != paths_added.end(); ++it) {
        std::string curr_path = *it;
        if(remove(get_path(curr_path))) {
            std::cout << "failed to remove " << curr_path << std::endl;
        }
    }
    paths_added.clear();
}
