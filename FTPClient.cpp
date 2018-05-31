#include <set>
#include "FTPClient.h"

FTPClient::FTPClient() = default;

string FTPClient::getCurrentPath() {
    string realPath;
    char *temp = realpath("./", nullptr);

    if (temp != nullptr) {
        realPath.assign(temp);
        free(temp);
    }

    return realPath;
}

int FTPClient::open(const vector<string> &args) {
    //TODO: handle connection timeout

    if (control.isConnected()) {
        cout << "Already connected, use close first\n";
        return -1;
    }

    string serverName;
    int port = DEFAULT_PORT;
    string response_str;
    int response_code;

    regex number("^[0-9]{1,5}$");    //Pattern for 1 to 5 digit

    serverName = args[0];

    if (args.size() == 2) {
        if (regex_match(args[1], number)) port = stoi(args[1]);
        else goto bad_port;
    }

    if (!control.setup(serverName, port)) {
        cout << "Can not connect to server\n";
        return -1;
    }

    response_str = control.Receive();
    if (verbose_mode) cout << response_str;

    if (response_str[0] < '1' || response_str[0] > '5') {
        cout << "Connection established, but this is not a ftp connection\nClose connection\n";
        control.close_connection();
        return -1;
    }

    /*ICP
      120			   Service ready in nnn minutes
         220		   Service ready for new user
      220
      421              Service not available, closing TELNET connection
    */
    response_code = stoi(response_str);
    if (response_code == 120) {
        response_str = control.Receive();    //Wait until Receive ok message
        response_code = stoi(response_str);
    }
    if (response_code == 220) cout << "Connected to " << serverName << "\n";
    else if (response_code == 421) cout << "Service not available\n";

    return response_code;

    bad_port:
    cout << "Bad port number\n";
    goto usage;

    usage:
    cout << "Usage: open host-name [port]\n";
    return -1;
}

int FTPClient::quit() {
    if (!control.isConnected()) return -1;

    int response_code;

    control.Send("QUIT\r\n");
    response_code = receive_response_from_server();

    /*QUIT
         221			Closing connection
         500			Syntax error
    */
    if (response_code == 221) {
        control.close_connection();
    }

    return response_code;
}

FTPClient::~FTPClient() {
    control.close_connection();
}

int FTPClient::put(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty())
        return -1;

    int response_code;
    string relativeFileName;
    string localFileName;
    string remoteFileName;

    relativeFileName = args[0];
    if (args.size() == 2) remoteFileName = args[1];
    else remoteFileName = args[0];

    localFileName = getAbsolutePath(relativeFileName);

    if (!isExist(localFileName)) {
        cout << "Local: " << localFileName << ": " << "No such file or directory.\n";
        return -1;
    }

    FILE *input;
    input = fopen(localFileName.c_str(), "rb");
    if (input == nullptr) {
        cout << "Can not open local file\n";
        return -1;
    }

    fseek(input, 0, SEEK_END);
    long fileSize = ftell(input);
    fseek(input, 0, SEEK_SET);

    void *data_connection;
    passive_mode ? data_connection = new TCPClient : data_connection = new TCPServer;

    if (!establish_data_connection(data_connection)) {
        cout << "Can not establish data connection\n";
        return -1;
    };

    control.Send("STOR " + remoteFileName + "\r\n");
    response_code = receive_response_from_server();
    if (response_code == 150) {
        char buff[BUFSIZE];
        long bytesSent = 0;
        if (passive_mode) {
            auto *data = (TCPClient *) data_connection;
            while (!feof(input)) {
                int nBytes = fread(buff, 1, BUFSIZE, input);
                bytesSent += data->Send(buff, nBytes);
                if (bytesSent > 0) printProgress(bytesSent * 1.0 / fileSize);
            }
            data->close_connection();
        } else {
            auto *data = (TCPServer *) data_connection;
            while (!feof(input)) {
                int nBytes = fread(buff, 1, BUFSIZE, input);
                bytesSent += data->Send(buff, nBytes);
                if (bytesSent > 0) printProgress(bytesSent * 1.0 / fileSize);
            }
            data->close_connection();
        }

        if (bytesSent > 0) cout << endl;

        response_code = receive_response_from_server();
    }

    fclose(input);
    return response_code;
}

string FTPClient::getAbsolutePath(const string &relativePath) {
    string temp = relativePath;
    string fileName;
    string absolutePath = getCurrentPath();

    //Get File/Folder name
    for (int i = temp.length() - 1; i >= 0; i--) {
        if (temp[i] != '/') {
            fileName.insert(fileName.begin(), temp[i]);
            temp.pop_back();
        } else break;
    }

    if (relativePath[0] == '/') {
        for (int i = temp.length() - 1; i >= 1; i--) {
            if (temp[i] != '.' && !(temp[i] == '/' && temp[i - 1] == '.') && !(temp[i] == '/' && fileName[0] == '/')) {
                fileName.insert(fileName.begin(), temp[i]);
                temp.pop_back();
            } else temp.pop_back();;
        }

        return "/" + fileName;
    }
    else if (relativePath[0] == '~') {
        absolutePath = "/home/";
        string name(getlogin());
        absolutePath += name;

        temp.erase(0, 2);
    }

    int count = 0;
    while (!temp.empty()) {
        char ch = temp[temp.length() - 1];
        if (ch == '/') {
            if (count == 2 && !absolutePath.empty()) {
                while (absolutePath[absolutePath.length() - 1] != '/') {
                    absolutePath.pop_back();
                }
                absolutePath.pop_back();
            }
            count = 0;
        } else if (ch == '.') {
            count++;
        }
        temp.pop_back();
    }

    return absolutePath + "/" + fileName;
}

inline bool FTPClient::isExist(const string &fileName) {
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

int FTPClient::pwd() {
    if (!control.isConnected()) {
        cout << "Not connected \n";
        return -1;
    }
    string response_str;
    int response_code;

    control.Send("PWD\r\n");
    response_str = control.Receive();

    response_code = stoi(response_str);
    if (response_code == 257) {
        cout << response_str;
    } else {
        //TODO LOI GUI LENH PWD
    }

    return response_code;
}

int FTPClient::list(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    void *data_connection;
    passive_mode ? data_connection = new TCPClient : data_connection = new TCPServer;

    if (!establish_data_connection(data_connection)) {
        cout << "Can not establish data connection\n";
        return -1;
    };

    if (args.empty()) {
        control.Send("LIST\r\n");
    } else {
        control.Send("LIST " + args[0] + "\r\n");
    }

    int response_code = receive_response_from_server();

    if ((response_code == 150) || (response_code == 125)) {
        string datalist;
        if (passive_mode) {
            auto *data = (TCPClient *) data_connection;
            datalist = data->Receive();
            data->close_connection();
        } else {
            auto *data = (TCPServer *) data_connection;
            datalist = data->Receive();
            data->close_connection();
        }
        cout << datalist << endl;
    }

    return receive_response_from_server();
}

int FTPClient::lcd(const vector<string> &args) {
    if (args.empty()) {
        cout << "Local directory now " << getCurrentPath() << endl;
    } else if (isExist(args[0]) == 1) {
        int n = chdir(args[0].c_str());
        cout << "Local directory now " << args[0] << endl;
        return n;
    } else {
        cout << "No such directory\n";
    }
    return 0;
}

int FTPClient::cd(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty())
        return -1;

    int response_code;

    control.Send("CWD " + args[0] + "\r\n");
    response_code = receive_response_from_server();

    if (response_code == 250) {
        cout << "Directory successfully changed.\n";
        return response_code;
    } else {
        cout << "Failed to change directory.\n";
    }
    return response_code;
}

int FTPClient::help(const vector<string> &args) {

    if (args.empty()) {
        cout << "Commands are:\n"
                "\t!\t\t?\t\tcd\n"
                "\tdelete\t\tdir\t\texit\n"
                "\tget\t\thelp\t\tlcd\n"
                "\tls\t\tmdelete\t\tmget\n"
                "\tmput\t\topen\t\tpassive\n"
                "\tput\t\tpwd\t\tquit\n"
                "\tuser\t\tverbose\n";
    } else {
        cout << "\t";
        if (args[0] == "user") {
            cout << "user\tlogin to FTP Server\n";
        } else if ((args[0] == "ls") || (args[0] == "dir")) {
            cout << args[0] + "\tlist contents of remote directory\n";
        } else if (args[0] == "put") {
            cout << "put\tput a file to remote directory\n";
        } else if (args[0] == "get") {
            cout << "get\tretrieve a file from remote directory\n";
        } else if (args[0] == "mput") {
            cout << "mput\tput multiple files to remote directory\n";
        } else if (args[0] == "mget") {
            cout << "mget\tretrieve multiple files from remote directory\n";
        } else if (args[0] == "cd") {
            cout << "cd\tchange remote working directory\n";
        } else if (args[0] == "lcd") {
            cout << "lcd\tchange local working directory\n";
        } else if (args[0] == "delete") {
            cout << "delete\tdelete a remote file\n";
        } else if (args[0] == "mdelete") {
            cout << "mdelete\tdelete multiple files\n";
        } else if (args[0] == "mkdir") {
            cout << "mkdir\tmake a directory on the remote machine\n";
        } else if (args[0] == "rmdir") {
            cout << "rmdir\tremove an empty directory on the remote machine\n";
        } else if (args[0] == "pwd") {
            cout << "pwd\tprint working directory on remote machine\n";
        } else if (args[0] == "passive") {
            cout << "passive\tenter passive transfer mode\n";
        } else if (args[0] == "verbose") {
            cout << "verbose\ttoggle verbose\n";
        } else if ((args[0] == "quit") || (args[0] == "exit") || (args[0] == "!")) {
            cout << args[0] + "\tterminate ftp session and exit\n";
        } else if ((args[0] == "help") || (args[0] == "?")) {
            cout << args[0] + "\tshow this help\n";
        } else if ((args[0] == "open")) {
            cout << args[0] + "\topen connection\n";
        } else {
            cout << "?Invalid help command " + args[0] << endl;
        }
    }

return 0;
}

void FTPClient::passive() {
    passive_mode = !passive_mode;
    if (passive_mode) cout << "Passive mode on\n";
    else cout << "Passive mode off\n";
}

void FTPClient::verbose() {
    verbose_mode = !verbose_mode;
    if (verbose_mode) cout << "Verbose on\n";
    else cout << "Verbose off\n";
}

int FTPClient::delete_cmd(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty()) return -1;

    int response_code;

    control.Send("DELE " + args[0] + "\r\n");
    response_code = receive_response_from_server();

    if (response_code == 250) {
        cout << "Delete operation successful\n";
    } else if ((response_code == 450) || (response_code == 550)) {
        cout << "Delete operation failed\n";
    }

    return response_code;
}

int FTPClient::mput(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    for (const auto &arg : args) {
        vector<string> temp;
        temp.push_back(arg);
        put(temp);
        temp.clear();
    }
}

int FTPClient::get(const vector<string> &args)
{
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty())
        return -1;

    FILE *output;
    if (args.size() == 1) {
        output = fopen(args[0].c_str(), "wb");
        cout << "Local: " << args[0] << "\n" << "Remote: " << args[0] << endl;
    } else if (args.size() == 2) {
        output = fopen(args[1].c_str(), "wb");
        cout << "Local: " << args[1] << "\n" << "Remote: " << args[0] << endl;
    }

    void *data_connection;
    passive_mode ? data_connection = new TCPClient : data_connection = new TCPServer;

    if (!establish_data_connection(data_connection)) {
        cout << "Can not establish data connection\n";
        return -1;
    };

    control.Send("RETR " + args[0] + "\r\n");
    int code = receive_response_from_server();

    if ((code == 150) || ((code == 125))) {
        char buff[BUFSIZE];
        clock_t t1, t2;
        int n, nSum = 0;

        t1 = clock();

        if (passive_mode) {
            auto *data = (TCPClient *) data_connection;

            while ((n = data->Receive(buff, BUFSIZE)) > 0) {
                nSum += n;
                fwrite(buff, 1, n, output);
            }

            t2 = clock();
            data->close_connection();
        } else {
            auto *data = (TCPServer *) data_connection;

            while ((n = data->Receive(buff, BUFSIZE)) > 0) {
                nSum += n;
                fwrite(buff, 1, n, output);
            }

            t2 = clock();
            data->close_connection();
        }

        fclose(output);

        code = receive_response_from_server();
        if ((code == 226) || (code == 250)) {

            float diff = (t2 - t1) * 1.0 / CLOCKS_PER_SEC;
            float speed = (float) nSum / diff;
            cout << fixed;
            cout << nSum << " bytes received in " << setprecision(5) << diff << " secs" << " (" << speed / 1024
                 << "KB/s)\n";
            cout << code << " Transfer complete\n";

        }
    }

    return code;//receive_response_from_server();
}


int FTPClient::mdelete(const vector<string> &args)
{
    if (!control.isConnected()) {
        cout << "Not connected\n";
        return -1;
    }

    for (const auto &arg : args) {
        vector<string> temp;
        temp.push_back(arg);
        delete_cmd(temp);
        temp.clear();
    }

    return 0;
}

int FTPClient::mkdir(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty())
        return -1;

    control.Send("MKD " + args[0] + "\r\n");

    return receive_response_from_server();
}

int FTPClient::rmdir(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty())
        return -1;

    control.Send("RMD " + args[0] + "\r\n");

    return receive_response_from_server();
}

int FTPClient::mget(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty())
        return -1;

    if (args[0] == "*" && args.size() == 1) {
        void *data_connection;
        passive_mode ? data_connection = new TCPClient : data_connection = new TCPServer;

        if (!establish_data_connection(data_connection)) {
            cout << "Can not establish data connection\n";
            return -1;
        };

        control.Send("NLST " + args[0] + "\r\n");
        int code = receive_response_from_server();

        if ((code == 150) || (code == 125)) {
            string datalist;
            if (passive_mode) {
                auto *data = (TCPClient *) data_connection;
                datalist = data->Receive();
                data->close_connection();

                delete[]data;
            } else {
                auto *data = (TCPServer *) data_connection;
                datalist = data->Receive();
                data->close_connection();

                delete[]data;
            }
            receive_response_from_server();

            vector<string> nlist;
            string nametemp;
            for (int i = 0; i < datalist.length(); i++) {
                if (datalist[i] != 0x0d) {
                    nametemp.push_back(datalist[i]);
                } else {
                    i++;
                    nlist.push_back(nametemp);
                    nametemp = "";
                    get(nlist);
                    nlist.clear();
                }
            }
        }
    } else {
        for (const auto &arg : args) {
            vector<string> temp;
            temp.push_back(arg);
            temp.push_back(arg);
            get(temp);
            temp.clear();
        }
    }

    return 0;
}

bool FTPClient::establish_data_connection(void *data_connection) {

    string response_str;
    int response_code;

    if (passive_mode) {
        auto *data = (TCPClient *) data_connection;

        //Enter passive mode
        control.Send("PASV\r\n");
        response_str = control.Receive();
        if (verbose_mode) cout << response_str;
        response_code = stoi(response_str);
        if (response_code != 227) {
            //TODO LOI GUI LENH PASV
        } else {
            string address;
            string port_low, port_high;
            int port;
            regex ipport("(\\d{1,3},\\d{1,3},\\d{1,3},\\d{1,3}),(\\d{1,3}),(\\d{1,3})");
            smatch sm;

            regex_search(response_str, sm, ipport);
            address = sm[1];
            port_high = sm[2];
            port_low = sm[3];

            for (int i = 0; i < address.size(); i++) {
                if (address[i] == ',') address.replace(i, 1, ".");
            }
            port = stoi(port_high) * 256 + stoi(port_low);

            return data->setup(address, port);
        }
    } else {
        auto *data = (TCPServer *) data_connection;
        data->wait_for_connection();

        //Send port information to server
        control.Send("PORT " + control.get_client_address() + "," + data->get_server_port() + "\r\n");
        response_str = control.Receive();
        if (verbose_mode) cout << response_str;

        response_code = stoi(response_str);

        return response_code == 200;
    }
}

int FTPClient::receive_response_from_server() {
    string response_str = control.Receive();
    int response_code = stoi(response_str);

    if (verbose_mode) cout << response_str;

    return response_code;
}

void FTPClient::setPassive() {
    passive_mode = true;
}

void FTPClient::setVerbose() {
    verbose_mode = true;
}

int FTPClient::user(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty())
        return -1;

    control.Send("USER " + args[0] + "\r\n");
    return receive_response_from_server();
}

int FTPClient::pass(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    if (args.empty())
        return -1;

    control.Send("PASS " + args[0] + "\r\n");
    return receive_response_from_server();
}

void FTPClient::printProgress(const double &percentage) {
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}