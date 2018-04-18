#include "FTPClient.h"

enum Commands {
	OPEN = 0,
    LOGIN,
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
    VERBOSE,
    NOT_CMD
};

struct Request {
	Commands command;
	vector<string> arg;
};

Request ReadRequest();

void help();

int main(int argc, char **argv) {
    FTPClient ftp;

    char c;
    string ip, port;

    while ((c = getopt(argc, argv, "a:Ppvh")) != -1) {
        switch (c) {
            case 'p':
                ftp.setPassive();
                break;
            case 'v':
                ftp.setVerbose();
                break;
            case 'a':
                ip.assign(optarg);
                break;
            case 'P':
                port.assign(optarg);
                break;
            case 'h':
            case '?':
                help();
                return 0;
            default:
                break;
        }
    }

    if (!ip.empty()) {
        vector<string> arg;
        arg.push_back(ip);
        if (!port.empty()) arg.push_back(port);

        if (ftp.open(arg) != -1) {
            string user, pass;
            vector<string> arg;
            cout << "Enter username: ";
            cin >> user;
            arg.push_back(user);
            cout << "Enter password: ";
            cin >> pass;
            cin.ignore();
            arg.push_back(pass);

            ftp.login(arg);
        }
    }

    while (true) {
        Request request = ReadRequest();

        switch (request.command) {
            case NOT_CMD:
                ftp.help(vector<string>());
                break;
            case LOGIN:
                ftp.login(request.arg);
                break;
            case OPEN:
                if (ftp.open(request.arg) != -1) {
                    string user, pass;
                    vector<string> arg;
                    cout << "Enter username: ";
                    cin >> user;
                    arg.push_back(user);
                    cout << "Enter password: ";
                    cin >> pass;
                    cin.ignore();
                    arg.push_back(pass);

                    ftp.login(arg);
                }
                break;
            case CD:
                ftp.cd(request.arg);
                break;
            case LCD:
                ftp.lcd(request.arg);
                break;
            case DELETE:
                ftp.delete_cmd(request.arg);
                break;
            case MDELETE:
                ftp.mdelete(request.arg);
                break;
            case PUT:
                ftp.put(request.arg);
                break;
            case MPUT:
                ftp.mput(request.arg);
                break;
            case LIST:
                ftp.list(request.arg);
                break;
            case GET:
                ftp.get(request.arg);
                break;
            case MGET:
                ftp.mget(request.arg);
                break;
            case MKDIR:
                ftp.mkdir(request.arg);
                break;
            case RMDIR:
                ftp.rmdir(request.arg);
                break;
            case PWD:
                ftp.pwd();
                break;
            case PASSIVE:
                ftp.passive();
                break;
            case VERBOSE:
                ftp.verbose();
                break;
            case QUIT:
                ftp.quit();
                return 0;
        }
    }
}

Request ReadRequest() {
    string request_str;
    Request request;
    vector<string> temps;
    string temp;

    do {
        cout << "ftp> ";
        getline(cin, request_str);
    } while (request_str.empty());

    for (int i = 0; i <= request_str.length(); i++) {
        if (request_str[i] != ' ' && i < request_str.length()) {
            temp.push_back(request_str[i]);
        } else {
            if (!temp.empty()) {
                temps.push_back(temp);
                temp.clear();
            }
        }
    }

    if (temps[0] == "open") {
        request.command = OPEN;
    } else if (temps[0] == "login") {
        request.command = LOGIN;
    } else if (temps[0] == "ls" || temps[0] == "dir") {
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
    } else if (temps[0] == "verbose") {
        request.command = VERBOSE;
    } else if (temps[0] == "quit" || temps[0] == "exit" || temps[0] == "!") {
        request.command = QUIT;
    } else if (temps[0] == "help" || temps[0] == "?") {
        request.command = HELP;
    } else {
        request.command = NOT_CMD;
    }

    if (temps.size() > 1) {
        for (int i = 1; i < temps.size(); i++) {
            request.arg.push_back(temps[i]);
        }
    }
    return request;
}

void help() {
    cout << "Usage: ftp [-vph] -a [Host name] -P [Port]\n"
            "\t   -h: show this help\n"
            "\t   -p: enable passive mode\n"
            "\t   -v: verbose mode\n";
}
