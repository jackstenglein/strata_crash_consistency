#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <map>
#include <vector>

// Represents a snapshot of single file.
class FileSnapshot {
	int status;
	int error;
	struct stat snapshot;

  public:
    FileSnapshot(const std::string);
	FileSnapshot(std::ifstream&);
	bool operator ==(const FileSnapshot other) const;
	void printState(std::ostream&) const;
	void writeToFile(std::ofstream&) const;
};

// Represents a snapshot of the filesystem.
class FSSnapshot {
	std::map<std::string, FileSnapshot> snapshots;
	std::string mountDir;
  public:
    FSSnapshot(std::string, const std::vector<std::string>&);
	FSSnapshot(std::string filename);
	bool operator==(const FSSnapshot other) const;
	void printState(std::ostream&) const;
	void writeToFile(std::string filename) const; 
};
