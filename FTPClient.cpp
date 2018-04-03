#include "FTPClient.h"

FTPClient::FTPClient()
{
	current_path = getexepath();
	isRunning = true;
}

FTPClient::FTPClient(const string &serverName, uint16_t port)
{
	current_path = getexepath();
	isRunning = true;
	
	//TODO: Khởi tạo kết nối
}

int FTPClient::setexepath(const string &path)
{
	return chdir(path.c_str());
}

string FTPClient::getexepath()
{
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
}

void FTPClient::open(const string &serverName, uint16_t port)
{
    tcp.setup(serverName, port);
    string rep = tcp.receive();
    cout << rep;
}

void FTPClient::list()
{

}

void FTPClient::put(const string &localFile, const string &remoteFile)
{

}

void FTPClient::mput(const vector<string> &localFiles, const vector<string> &remoteFiles)
{

}

void FTPClient::get(const string &remoteFile, const string &localFile)
{

}

void FTPClient::mget(const vector<string> &remoteFiles, const vector<string> &localFiles)
{

}

void FTPClient::cd(const string &remoteDirectory)
{

}

void FTPClient::lcd(const string &localDirectory)
{

}

void FTPClient::delete_(const string &remoteFile)
{

}

void FTPClient::mdelete(const string &remoteFiles)
{

}

void FTPClient::mkdir(const string &directoryName)
{

}

void FTPClient::rmdir(const string &directoryName)
{

}

void FTPClient::pwd()
{

}

void FTPClient::passive()
{

}

void FTPClient::help()
{

}

void quit()
{

}

void FTPClient::run()
{
    open("speedtest.tele2.net", 21);
}

FTPClient::~FTPClient()
{
	
}