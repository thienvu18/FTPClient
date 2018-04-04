#include "FTPClient.h"

enum Commands {
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

struct Request {
	Commands command;
	vector<string> arg;
};

Request ReadRequest();

int main(int argc, char** argv)
{
	FTPClient ftp;

	string name;
	string port;
	cin >> name;
	cin >> port;
	vector<string> arg;
	arg.push_back(name);
	if (port != "0") arg.push_back(port);

	ftp.open(arg);
	ftp.open(arg);
	ftp.quit();
	
    return 0;
}
