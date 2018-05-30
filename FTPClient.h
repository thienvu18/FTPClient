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
#include <pthread.h>
#include <iomanip>

#include "lib/TCPClient.h"
#include "lib/TCPServer.h"

#define TIMEOUT 500
#define DEFAULT_PORT 21

using namespace std;

class FTPClient
{
private:
    bool verbose_mode = true;
    bool passive_mode = false;
    TCPClient control;

	int setCurrentPath(const string &path);
	string getCurrentPath();

	//string getParentPath(int nLevels);	//Unused
	string getAbsolutePath(const string &fileName);

	inline bool isExist(const string &fileName);

    void *wait_for_connecting(TCPServer &data, void *threadid);

    bool establish_data_connection(void *data_connection);

    int receive_response_from_server();

public:
	FTPClient();

    void setPassive();

    void setVerbose();

	int open(const vector<string> &args);

    int user(const vector<string> &args);

    int pass(const vector<string> &args);

	int list(const vector<string> &args);

	int put(const vector<string> &args);

    int mput(const vector<string> &args);
	int get(const vector<string> &args);

    int mget(const vector<string> &args);
	int cd(const vector<string> &args);

	int lcd(const vector<string> &args);

	int delete_cmd(const vector<string> &args);

	int mdelete(const vector<string> &args);

	int mkdir(const vector<string> &args);

	int rmdir(const vector<string> &args);

    int pwd();

    void passive();

    void verbose();


	int help(const vector<string> &args);
	int quit();

	~FTPClient();
};

#endif