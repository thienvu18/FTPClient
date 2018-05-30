//
// Created by thienvu on 5/30/18.
//
#include <string>

#ifndef FTPCLIENT_DEFINE_H
#define FTPCLIENT_DEFINE_H

enum COMMANDS {
    OPEN = 0,
    USER,
    LIST,
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
    QUIT,
    HELP,
    VERBOSE,
    NOT_CMD
};

struct Command {
    string cmd;
    int argCount;
    vector<string> argList;

    explicit Command(const string &cmd, int argCount = 0, vector<string> argList = {}) {
        this->cmd = cmd;
        this->argCount = argCount;
        this->argList = std::move(argList);
    }
};

struct Request {
    COMMANDS command;
    vector<string> arg;
};

vector<Command> COMMAND_LIST = {
        Command("open", 1, {"Hostname: ", "Port: "}),
        Command("user", 1, {"User name: "}),
        Command("ls"),
        Command("dir"),
        Command("put", 1, {"(local-file)", "(remote-file)"}),
        Command("mput", 2, {"(local-file)", "(remote-file)"}),
        Command("get", 1, {"(remote-file)", "(local-file)"}),
        Command("mget", 1, {"(remote-file)", "(local-file)"}),
        Command("cd", 1, {"(remote-directory)"}),
        Command("lcd"),
        Command("delete", 1, {"(remote-file)"}),
        Command("mdelete", 1, {"(remote-file)"}),
        Command("mkdir", 1, {"(directory-name)"}),
        Command("rmdir", 1, {"(directory-name)"}),
        Command("pwd"),
        Command("help"),
        Command("?"),
        Command("quit"),
        Command("exit"),
        Command("!"),
        Command("passive"),
        Command("verbose")
};

#endif //FTPCLIENT_DEFINE_H
