#include <vector>
#include <map>


class AbstractAceRunner {
	typedef int (AbstractAceRunner::*AceActionHandler)(std::vector<std::string>&);
	typedef std::map<std::string, AceActionHandler> handler_map;

	private:
		int test_fd;
		handler_map actionHandlers;
		std::map<std::string, int> fileDescriptors;
		std::string getFilePath(std::string file);
		virtual int handle_checkpoint(std::vector<std::string>& tokens) = 0;
		int handle_close(std::vector<std::string>& tokens);
		int handle_falloc(std::vector<std::string>& tokens);
		int handle_fdatasync(std::vector<std::string>& tokens);
		int handle_fsync(std::vector<std::string>& tokens);
		int handle_mkdir(std::vector<std::string>& tokens);
		int handle_open(std::vector<std::string>& tokens);
		int handle_opendir(std::vector<std::string>& tokens);
		int handle_remove(std::vector<std::string>& tokens);
		int handle_rename(std::vector<std::string>& tokens);
		int handle_sync(std::vector<std::string>& tokens);
		int handle_truncate(std::vector<std::string>& tokens);
		int handle_unlink(std::vector<std::string>& tokens);
		int handle_write(std::vector<std::string>& tokens);
		int parse_falloc_flags(std::string flags);
		int parse_open_flags(std::string flags);
	protected:
		const std::vector<std::string> getAllFilePaths(void);
		std::string testDir;
	public:
	  	AbstractAceRunner(std::string testDir);
		int handle_action(std::vector<std::string>& tokens);
		virtual void reset(void);
};

class OracleAceRunner : public AbstractAceRunner {
	private:
		std::string outputFile;
		virtual int handle_checkpoint(std::vector<std::string>& tokens);
	public:
		OracleAceRunner(std::string, std::string);
};

class CrashAceRunner : public AbstractAceRunner {
	private:
		bool shouldCrash;
		virtual int handle_checkpoint(std::vector<std::string>& tokens);
	public:
		CrashAceRunner(std::string, std::string);
		virtual void reset(void);
};
