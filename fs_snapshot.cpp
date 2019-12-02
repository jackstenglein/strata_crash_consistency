#include <sys/stat.h>
#include <string>
#include <map>
#include <cstring>
#include <iostream>
#include <fstream>

// Represents a snapshot of single file.
class FileSnapshot {
	int status;
	int error;
	struct stat snapshot;

  public:
    FileSnapshot(const std::string);
	FileSnapshot(std::ifstream&);
	bool operator ==(const FileSnapshot other) const;
	void printState() const;
	void writeToFile(std::ofstream&) const;
};

// Takes a file path and calls stat to save the file information.
FileSnapshot::FileSnapshot(const std::string path) {
	status = stat(path.c_str(), &snapshot);
	if (status == -1) {
		error = errno;
	} else {
		error = 0;
	}
}

// Take an ifstream and reads in the file snapshot information.
FileSnapshot::FileSnapshot(std::ifstream& in) {
	in.read(reinterpret_cast<char*>(&status), sizeof(int));
	in.read(reinterpret_cast<char*>(&error), sizeof(int));
	in.read(reinterpret_cast<char*>(&snapshot), sizeof(struct stat));
}

// Compares two FileSnapshot objects. They are equal if their filepaths
// and their snapshots are the same.
bool FileSnapshot::operator==(const FileSnapshot other) const {
	if (status != other.status || error != other.error) {
		return false;
	}
	return ! memcmp(&snapshot, &other.snapshot, sizeof(struct stat)); // TODO: this might not work b/c of access time
}

// Prints the fields of the file's stat struct at the time of the snapshot.
void FileSnapshot::printState() const {
	printf("\t\tDevice: %d\n", snapshot.st_dev);
	printf("\t\tInode: %d\n", snapshot.st_ino);
	printf("\t\tMode: %d\n", snapshot.st_mode);
	printf("\t\tHard Links: %d\n", snapshot.st_nlink);
	printf("\t\tUser ID: %d\n", snapshot.st_uid);
	printf("\t\tGroup ID: %d\n", snapshot.st_gid);
	printf("\t\tRdev: %d\n", snapshot.st_rdev);
	printf("\t\tSize: %d\n", snapshot.st_size);
	printf("\t\tBlock size: %d\n", snapshot.st_blksize);
	printf("\t\tBlocks: %d\n", snapshot.st_blocks);
	printf("\t\tAccess time: %lld.%.9ld\n", (long long)snapshot.st_atim.tv_sec, snapshot.st_atim.tv_nsec);
	printf("\t\tModified time: %lld.%.9ld\n", (long long)snapshot.st_mtim.tv_sec, snapshot.st_mtim.tv_nsec);
	printf("\t\tChange time: %lld.%.9ld\n", (long long)snapshot.st_ctim.tv_sec, snapshot.st_ctim.tv_nsec);
}

// Writes the file snapshot info to the given output stream.
void FileSnapshot::writeToFile(std::ofstream& out) const {
	out.write(reinterpret_cast<const char*>(&status), sizeof(int));
	out.write(reinterpret_cast<const char*>(&error), sizeof(int));
	out.write(reinterpret_cast<const char*>(&snapshot), sizeof(struct stat));
}


// Represents a snapshot of the filesystem.
class FSSnapshot {
	std::map<std::string, FileSnapshot> snapshots;
  public:
    FSSnapshot(const std::set<std::string>&);
	FSSnapshot(std::string filename);
	bool operator==(const FSSnapshot other) const;
	void printState() const;
	void writeToFile(std::string filename) const; 
};

// Takes a map of filepaths to file descriptors and creates a snapshot of the 
// files in the map.
FSSnapshot::FSSnapshot(const std::set<std::string>& paths) {
	for (auto it = paths.cbegin(); it != paths.cend(); ++it) {
		FileSnapshot* snapshot = new FileSnapshot(*it);
		snapshots.insert(std::pair<std::string, FileSnapshot>(*it, *snapshot));
	}
}

FSSnapshot::FSSnapshot(std::string filename) {
	std::ifstream infile;
	std::string snapshotName;
	infile.open(filename);
	if ( ! infile.is_open()) {
		std::cout << "Failed to open input file" << std::endl;
		return;
	}

	while(std::getline(infile, snapshotName)) {
		FileSnapshot snapshot(infile);
		snapshots.insert(std::pair<std::string, FileSnapshot>(snapshotName, snapshot));
	}

	infile.close();
}

// Compares two FSSnapshot objects. They are equal if they have the same FileSnapshot
// objects.
bool FSSnapshot::operator==(const FSSnapshot other) const {
	return snapshots == other.snapshots;
}

// Prints the state of each file in the snapshot.
void FSSnapshot::printState() const {
	printf("*** BEGIN FS SNAPSHOT ***\n");
	for (auto it = snapshots.cbegin(); it != snapshots.cend(); ++it) {
		printf("\n\t%s\n", it->first.c_str());
		it->second.printState();
	}
	printf("\n*** END FS SNAPSHOT ***\n");
}

// Writes the FSSnapshot to the specified file.
void FSSnapshot::writeToFile(std::string filename) const {
	std::ofstream outfile;
	outfile.open(filename, std::ios::out|std::ios::trunc);
	if ( ! outfile.is_open()) {
		std::cout << "Failed to open output file" << std::endl;
		return;
	}

	for (auto it = snapshots.cbegin(); it != snapshots.cend(); ++it) {
		outfile << it->first << '\n';
		it->second.writeToFile(outfile);
	}	

	outfile.close();
}
