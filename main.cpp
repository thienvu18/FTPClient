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
    HELP,    //help, ?
    NOT_CMD
};

struct Request {
	Commands command;
	vector<string> arg;
};

Request ReadRequest();

//int main(int argc, char** argv)
//{
//	FTPClient ftp;
//	Request request;
//
//	if (argc > 1)
//    {
//        //todo: parse command line
//    }
//
//    while (true) {
//        request = ReadRequest();
//
//        switch (request.command) {
//            case NOT_CMD:
//                ftp.help(vector<string>());
//                break;
//            case OPEN:
//                if (ftp.open(request.arg) != -1)
//                {
//                    string user, pass;
//                    vector<string> arg;
//                    cout << "Enter username: ";
//                    cin >> user;
//                    arg.push_back(user);
//                    cout << "Enter password: ";
//                    cin >> pass;
//                    arg.push_back(pass);
//
//                    ftp.login(arg);
//                }
//                break;
//            case CD:
//                ftp.cd(request.arg);
//                break;
//            case LCD:
//                ftp.lcd(request.arg);
//                break;
//            case DELETE:
//                ftp.delete_cmd(request.arg);
//                break;
//            case MDELETE:
//                ftp.mdelete(request.arg);
//                break;
//            case PUT:
//                ftp.put(request.arg);
//                break;
//            case MPUT:
//                ftp.mput(request.arg);
//                break;
//            case HELP:
//                ftp.cd(request.arg);
//                break;
//            case QUIT:
//                return 0;
//        }
//    }
//
//    return 0;
//}

int main() //test
{
    FTPClient ftp;

    vector<string> server, user, file;
    server.push_back("127.0.0.1");
    user.push_back("thienvu");
    user.push_back("18022804");
    file.push_back("*");
    //file.push_back("/home/thienvu/thaivu.abc");

    ftp.open(server);
    ftp.login(user);
    ftp.mget(file);
    return 0;
}

Request ReadRequest() {
    string request_str;
    Request request;
    vector<string> temps;
    string temp;

    cout << "ftp> ";
    getline(cin, request_str);

    for (int i = 0; i <= request_str.length(); i++) {
        if (request_str[i] != ' ' && i < request_str.length()) {
            temp.push_back(request_str[i]);
        } else {
            if (temp.size() != 0) {
                temps.push_back(temp);
                temp.clear();
            }
        }
    }

    if (temps[0] == "open") {
        request.command = OPEN;
    } else if (temps[0] == "list" || temps[0] == "dir") {
        request.command = LIST;
    } else if (temps[0] == "put") {
        request.command = PUT;
    } else if (temps[0] == "mput") {
        request.command = MPUT;
    } else if (temps[0] == "get") {
        request.command = GET;
    } else if (temps[0] == "mget") {
        request.command = MGET;
    } else if (temps[0] == "cd") {
        request.command = CD;
    } else if (temps[0] == "lcd") {
        request.command = LCD;
    } else if (temps[0] == "delete") {
        request.command = DELETE;
    } else if (temps[0] == "mdelete") {
        request.command = MDELETE;
    } else if (temps[0] == "mkdir") {
        request.command = MKDIR;
    } else if (temps[0] == "rmdir") {
        request.command = RMDIR;
    } else if (temps[0] == "pwd") {
        request.command = PWD;
    } else if (temps[0] == "passive") {
        request.command = PASSIVE;
    } else if (temps[0] == "quit" || temps[0] == "exit" || temps[0] == "!") {
        request.command = QUIT;
    } else if (temps[0] == "help" || temps[0] == "?") {
        request.command = HELP;
    } else {
        request.command = NOT_CMD;
    }

    for (int i = 1; i < temps.size(); i++) {
        request.arg.push_back(temps[i]);
    }

    return request;
}