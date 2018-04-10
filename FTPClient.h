#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include <iostream>
#include <vector>
#include <string>
#include <climits>
#include <unistd.h>
#include <regex>
#include <sys/stat.h>
#include <fstream>
#include <string>

#include "lib/TCPClient.h"
#include "lib/TCPServer.h"

#define TIMEOUT 500
#define DEFAULT_PORT 21

using namespace std;

class FTPClient
{
private:
	string current_path;
	bool isRunning;
	bool verbose = true;
	bool passive_mode = true;
    TCPClient control;

	int setCurrentPath(const string &path);
	string getCurrentPath();

	//string getParentPath(int nLevels);	//Unused
public:
	string getAbsolutePath(const string &fileName);

	inline bool isExist(const string &fileName);


public:
	FTPClient();

	int open(const vector<string> &arg);

	int user(const vector<string> &arg);
    int password (const vector<string> &arg);
	int login(const vector<string> &arg);
	int list(const vector<string> &arg);
	int put(const vector<string> &arg);
//	int mput(const vector<string> &arg);//
//	int get(const vector<string> &arg);
//	int mget(const vector<string> &arg);//
	int cd(const vector<string> &args);
	int lcd(const vector<string> &arg);
//	int delete_cmd(const vector<string> &arg);
//	int mdelete(const vector<string> &arg);
//	int mkdir(const vector<string> &arg);
//	int rmdir(const vector<string> &arg);
    int pwd();//
//	int passive();
	int help(const vector<string> &arg);
	int quit();

	~FTPClient();
};

#endif