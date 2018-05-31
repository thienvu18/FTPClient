#include <utility>
#include "FTPClient.h"
#include "define.h"

Request ReadRequest();
void login(FTPClient &ftp, const string &userName = "");

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
            login(ftp);
        }
    }

    while (true) {
        Request request = ReadRequest();

        switch (request.command) {
            case NOT_CMD:
                ftp.help(vector<string>());
                break;
            case USER:
                if (request.arg.size() == 1)
                    login(ftp, request.arg[0]);
                else login(ftp);
                break;
            case OPEN:
                if (ftp.open(request.arg) != -1) {
                    login(ftp);
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
            case HELP:
                ftp.help(request.arg);
                break;
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

    bool flag = false;

    for (int i = 0; i <= request_str.length(); i++) {
        if (request_str[i] == '\"') {
            flag = !flag;
            continue;
        }

        if (request_str[i] != ' ' && i < request_str.length()) {
            temp.push_back(request_str[i]);
        } else if (request_str[i] == ' ' && flag == true) {
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
    } else if (temps[0] == "user") {
        request.command = USER;
    } else if (temps[0] == "ls" || temps[0] == "dir") {
        request.command = LIST;
        temps[0] == "ls";
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
        temps[0] == "quit";
    } else if (temps[0] == "help" || temps[0] == "?") {
        request.command = HELP;
        temps[0] == "help";
    } else {
        request.command = NOT_CMD;
    }

    for (auto &command : COMMAND_LIST) {
        if (temps[0] == command.cmd) {
            for (int j = 1; j < temps.size(); j++) {
                request.arg.push_back(temps[j]);
            }

            if (temps.size() < command.argCount + 1) {
                string additionArg;
                for (int j = temps.size() - 1; j < command.argList.size(); j++) {
                    cout << command.argList[j] << " ";
                    cin >> additionArg;
                    cin.ignore();
                    request.arg.push_back(additionArg);
                }
            }

            break;
        }
    }

    return request;
}

void help() {
    cout << "Usage: ftp [-vph] -a [Host name] -P [Port]\n"
            "\t   -h: show this help\n"
            "\t   -p: passive mode\n"
            "\t   -v: verbose mode\n";
}

void login(FTPClient &ftp, const string &userName) {
    string user, pass;
    vector<string> arg;

    if (userName.empty()) {
        cout << "Enter username: ";
        cin >> user;
        cin.ignore();
    } else user = userName;

    arg.push_back(user);

    if (ftp.user(arg) == 331) {
        arg.clear();
        pass.assign(getpass("Enter password: "));
        arg.push_back(pass);
        ftp.pass(arg);
    }
}