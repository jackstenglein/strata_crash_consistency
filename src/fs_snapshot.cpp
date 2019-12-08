#include "fs_snapshot.h"
#include <string>
#include <cstring>

/****************
 * FileSnapshot *
 ****************/

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
void FileSnapshot::printState(std::ostream& out) const {
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

// Writes the file snapshot info to the given output stream.
void FileSnapshot::writeToFile(std::ofstream& out) const {
	out.write(reinterpret_cast<const char*>(&status), sizeof(int));
	out.write(reinterpret_cast<const char*>(&error), sizeof(int));
	out.write(reinterpret_cast<const char*>(&snapshot), sizeof(struct stat));
}


/**************
 * FSSnapshot *
 **************/

// Takes a map of filepaths to file descriptors and creates a snapshot of the 
// files in the map.
FSSnapshot::FSSnapshot(const std::set<std::string>& paths) {
	for (auto it = paths.cbegin(); it != paths.cend(); ++it) {
		FileSnapshot* snapshot = new FileSnapshot(*it);
		snapshots.insert(std::pair<std::string, FileSnapshot>(*it, *snapshot));
	}
}

// Takes a path to a file previously written by FSSnapshot::writeToFile.
// The file is read in to create a FSSnapshot object.
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
void FSSnapshot::printState(std::ostream& out) const {
	out << "*** BEGIN FS SNAPSHOT ***\n";
	for (auto it = snapshots.cbegin(); it != snapshots.cend(); ++it) {
		out << "\n\t" << it->first.c_str() << "\n";
		it->second.printState(out);
	}
	out << "\n*** END FS SNAPSHOT ***\n";
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
