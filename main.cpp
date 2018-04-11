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

    vector<string> server, user,file;
    server.push_back("127.0.0.1");
    user.push_back("vannguyen");
    user.push_back("Kimtaehuyng95");
    //file.push_back("B1_5.jpg");
    //file.push_back("abc.jpg");
    file.push_back("/home/vannguyen/Downloads/bichvan");
    file.push_back("/home/vannguyen/Downloads/vanbich");
    ftp.open(server);
    ftp.login(user);
    //ftp.put(file);
    //ftp.pwd();
    ftp.get(file);
    return 0;
}
