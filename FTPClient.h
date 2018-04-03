#ifndef FTP_CLIENT_H_
#define FTP_CLIENT_H_

#include <iostream>
#include <vector>
#include <string>
#include <limits.h>
#include <unistd.h>
#include "lib/TCPClient.h"

#define TIMEOUT 500
#define DEFAULT_PORT 21

using namespace std;

enum Commands
{
    OPEN = 0,
    LIST,   //ls, dir
    PUT,
    MPUT,
    GET,
    MGET,
    CD,
    LCD,
    DELETE,
    MDELETE,
    MKDIR,
    RMDIR,
    PWD,
    PASSIVE,
    QUIT,   //quit, exit, !
    HELP    //help, ?
};

struct Request
{
    Commands command;
    vector<string> arg;
};

struct Response
{
    uint8_t code;
    string msg;
};

class FTPClient
{
private:
	string current_path;
	bool isRunning;
	bool verbose = true;
	TCPClient tcp;
    
    string getexepath();
	int setexepath(const string &path);

	Request ReadRequest();

	
	void open(const string &serverName, uint16_t port);
	void list();//
	void put(const string &localFile, const string &remoteFile);
	void mput(const vector<string> &localFiles, const vector<string> &remoteFiles);//
	void get(const string &remoteFile, const string &localFile);
	void mget(const vector<string> &remoteFiles, const vector<string> &localFiles);//
	void cd(const string &remoteDirectory);///
	void lcd(const string &localDirectory);//
	void delete_(const string &remoteFile);
	void mdelete(const string &remoteFiles);
	void mkdir(const string &directoryName);
	void rmdir(const string &directoryName);
	void pwd();//
	void passive();
	void help();//
	void quit();

public:
	FTPClient();
	FTPClient(const string &serverName, uint16_t port = DEFAULT_PORT);
	
	void run();
	
	~FTPClient();
};

#endif