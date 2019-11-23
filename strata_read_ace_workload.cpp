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

// using namespace std;

#define FLAGS_INDEX 2
#define TEST_DIR "j-lang-files" //"crashmonkey/code/tests/seq1/j-lang-files"


std::vector<std::string> tokenize(std::string str);
void run_test(std::string test_file, std::set<std::string> &unhandled_actions);
int handle_mkdir(std::vector<std::string> tokens);
int handle_open(std::vector<std::string> tokens);
int handle_truncate(std::vector<std::string> tokens);
int handle_write(std::vector<std::string> tokens);
const char* get_path(std::string file);
int parse_open_flags(std::string flags);

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
        perror("Could not open directory");
        return EXIT_FAILURE;
    }

    std::cout << "Unhandled Actions: ";
    for (std::set<std::string>::iterator it=unhandled_actions.begin(); it!=unhandled_actions.end(); ++it)
        std::cout << ' ' << *it << std::endl;
	return 0;
}

void run_test(std::string test_file, std::set<std::string> &unhandled_actions) {

    int err;
    int fd = -1;
	std::string line;
	std::ifstream infile(test_file);
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
            /**
            if(fsync(paths_to_fds[get_path(tokens[1])])) {
                std::cout << "Failed to fsync" << std::endl;
            }
            */
        } else if (action == "fdatasync") {
            std::cout << "fdatasync" << std::endl;
        } else if (action == "mkdir") {
            std::cout << "mkdir" << std::endl;
            if (handle_mkdir(tokens)) {
                std::cout << "Failed to create directory." << std::endl;
            }
        } else if (action == "open") {
            std::cout << "open" << std::endl;
            fd = handle_open(tokens);
            if (fd < 0) {
                std::cout << "Failed to open file." << std::endl;
            }
            std::cout << "opened file fd" << fd << std::endl;

        } else if (action == "opendir") {
            std::cout << "opendir" << std::endl;
            // tokens.emplace(FLAGS_INDEX, "O_DIRECTORY");
            // fd = handle_open(tokens);
            // if (fd < 0) {
            //     std::cout << "Failed to open directory." << std::endl;
            // }
        } else if (action == "close") {
            std::cout << "close" << std::endl;
            fd = paths_to_fds[get_path(tokens[1])];
            if (close(fd)) {
                std::cout << "Failed to close file." << std::endl;
            }
        } else if (action == "rename") {
            std::cout << "Renaming file " << std::endl;
        } else if (action == "truncate") {
            std::cout << "Truncating file " << std::endl;
            int result = handle_truncate(tokens);
            if(result) {
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
        } else {
            unhandled_actions.insert(action);
        }
    }    
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

int handle_mkdir(std::vector<std::string> tokens) {
    const int dir_index = 1;
    const int perm_index = 2;

    const char* c_dir_name = tokens[dir_index].c_str();
    int permissions = std::stoi(tokens[perm_index], nullptr, 8);
    std::cout << "Create directory " << c_dir_name << " with base 10 permissions " << permissions << std::endl;
    return mkdir(c_dir_name, (mode_t)permissions);
}

int handle_open(std::vector<std::string> tokens) {
    
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
    const int flags = 0 | O_RDWR;
    const int permissions = std::stoi("0777");
    int fd = open(file_path, flags, permissions);
    std::cout << "handle_write fd " << fd << std::endl;
    int count = stoi(tokens[3]);
    std::string s(count, '0');
    return write(fd, s.c_str(), count);
}


const char* get_path(std::string file) {
    // Ace only has a finite number of paths, so just check for each individually
    if (file == "Afoo") return "A/foo";
    if (file == "Bfoo") return "B/foo";
    if (file == "foo") return "foo";
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
