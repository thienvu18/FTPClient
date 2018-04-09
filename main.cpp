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


string getCurrentPath() {
    string realPath;
    char *temp = realpath("./", nullptr);

    if (temp != nullptr) {
        realPath.assign(temp);
        free(temp);
    }

    return realPath;
}

string getParrentPath(int nLevels) {
    string parrentDenotes;
    string realPath;

    for (int i = 0; i < nLevels; i++) parrentDenotes += "../";

    char *temp = realpath(parrentDenotes.c_str(), nullptr);

    if (temp != nullptr) {
        realPath.assign(temp);
        free(temp);
    }

    return realPath;
}


string getAbolutePath(const string &relativePath) {
    //TODO: xet truong hop /./, .././, ./../,
    string temp = relativePath;
    string fileName;
    string absolutPath = getCurrentPath();

    //Get File/Folder name
    for (int i = temp.length() - 1; i >= 0; i--) {
        if (temp[i] != '/') {
            fileName.insert(fileName.begin(), temp[i]);
            temp.pop_back();
        } else break;
    }

    if (relativePath[0] == '/') return "/" + fileName;
    else if (relativePath[0] == '~') temp.erase(0, 2);

    int count = 0;
    while (!temp.empty()) {
        char ch = temp[temp.length() - 1];
        if (ch == '/') {
            if (count == 2 && !absolutPath.empty()) {
                while (absolutPath[absolutPath.length() - 1] != '/') {
                    absolutPath.pop_back();
                }
                absolutPath.pop_back();
            }
            count = 0;
        } else if (ch == '.') {
            count++;
        }
        temp.pop_back();
    }

    return absolutPath + "/" + fileName;
}


int main(int argc, char** argv)
{
	FTPClient ftp;

//	string name;
//	string port;
//	cin >> name;
//	cin >> port;
//	vector<string> arg;
//	arg.push_back(name);
//	if (port != "0") arg.push_back(port);
//
//	ftp.open(arg);
//
//
//	ftp.open(arg);
//	ftp.quit();

//        string path ="~/main.cpp";
//        cout << getAbsolutePath(path);

    TCPClient client;
    client.setup("127.0.0.1", 21);
    cout << client.get_client_address();
	
    return 0;
}
