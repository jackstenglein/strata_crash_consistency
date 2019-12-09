#include "fs_snapshot.h"
#include <string>
#include <cstring>

// Compares two stat structs and returns true if the snapshots
// are equal. The inode, access time, modified time and change
// time fields are not compared.
bool snapshotsEqual(const struct stat* lhs, const struct stat* rhs) {
	if (lhs->st_dev != rhs->st_dev) return false;
	if (lhs->st_mode != rhs->st_mode) return false;
	if (lhs->st_nlink != rhs->st_nlink) return false;
	if (lhs->st_uid != rhs->st_uid) return false;
	if (lhs->st_gid != rhs->st_gid) return false;
	if (lhs->st_rdev != rhs->st_rdev) return false;
	if (lhs->st_size != rhs->st_size) return false;
	if (lhs->st_blksize != rhs->st_blksize) return false;
	if (lhs->st_blocks != rhs->st_blocks) return false;

	return true;
}

/****************
 * FileSnapshot *
 ****************/

// Takes a file path and calls stat to save the file information.
FileSnapshot::FileSnapshot(const std::string path) {
	memset(&snapshot, 0, sizeof(struct stat));
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

	// We can't use memcmp on the stat struct because
	// the access times will be different.
	return snapshotsEqual(&snapshot, &other.snapshot);
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
FSSnapshot::FSSnapshot(std::string _mountDir, const std::vector<std::string>& paths) {
	mountDir = _mountDir;
	for (auto it = paths.cbegin(); it != paths.cend(); ++it) {
		FileSnapshot* snapshot = new FileSnapshot(*it);
		std::string fileName = (*it).substr(mountDir.size());
		snapshots.insert(std::pair<std::string, FileSnapshot>(fileName, *snapshot));
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
		out << "\n\t" << mountDir << it->first.c_str() << "\n";
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
