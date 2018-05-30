#include <set>
#include "FTPClient.h"

FTPClient::FTPClient() {
}

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
    regex ip(
            R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)");
    regex url(R"(^(((ftp|http|https):\/\/)?((\w+)(\.\w+)+)+)$)");

    //Check number of args.
    if (args.empty() && args.size() > 2) goto usage;

    if (regex_match(args[0], ip) || regex_match(args[0], url)) serverName = args[0];
    else goto invalid_args;

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

    invalid_args:
    cout << "Invalid argument\n";
    goto usage;

    usage:
    cout << "Usage: open host-name [port]\n";
    return -1;
}

int FTPClient::quit() {
    if (!control.isConnected()) {
        cout << "Not connected";
        return -1;
    }

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

    //Check number of args.
    if (args.empty() && args.size() > 2) {
        cout << "Usage: put local_file [remote_file]\n";
        return -1;
    };

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

    void *data_connection;
    passive_mode ? data_connection = new TCPClient : data_connection = new TCPServer;

    if (!establish_data_connection(data_connection)) {
        cout << "Can not establish data connection\n";
        return -1;
    };

    control.Send("STOR " + remoteFileName + "\r\n");

    if (receive_response_from_server() == 150) {
        char buff[BUFSIZE];
        while (!feof(input)) {
            int nbytes = fread(buff, 1, BUFSIZE, input);

            if (passive_mode) {
                TCPClient *data = (TCPClient *) data_connection;
                data->Send(buff, nbytes);
                data->close_connection();
            } else {
                TCPServer *data = (TCPServer *) data_connection;
                data->Send(buff, nbytes);
                data->close_connection();
            }
        }
    }

    fclose(input);
    return receive_response_from_server();
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

int FTPClient::login(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    int response_code;

    control.Send("USER " + args[0] + "\r\n");
    response_code = receive_response_from_server();

    if (response_code == 331) {
        control.Send("PASS " + args[1] + "\r\n");
        response_code = receive_response_from_server();

        if (response_code == 230) {
            cout << "Login successful\n";
        } else if (response_code == 530) {
            cout << "Login incorrect\n";
            cout << "Login failed\n";
            //TODO LOI GUI LENH PASS
        }
    }

    return response_code;
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
    if (verbose_mode) cout << response_str;

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

    //Check number of args.
    if (args.size() > 2) {
        cout << "Usage\n";
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

    if ((receive_response_from_server() == 150)||(receive_response_from_server() == 125)) {
        if (passive_mode) {
            TCPClient *data = (TCPClient *) data_connection;
            string datalist;
            datalist = data->Receive();
            cout << datalist;
            data->close_connection();
        } else {
            TCPServer *data = (TCPServer *) data_connection;
            string datalist;
            datalist = data->Receive();
            cout << datalist;
            data->close_connection();
        }
        int code = receive_response_from_server();
        if ((code == 226) || (code == 250)) {
            cout << "Directory send ok.\n";
            return code;
        } else if ((code== 425) || (code== 426) || (code== 451)) {
            cout << "Not connected\n";
        } else if (code == 450) {
            cout << "Requested file action not taken\n";
        } else {
            //TODO LOI GUI LENH LIST
        }
    } else{
        //TODO LOI GUI LENH LIST
    }
    return 0;
}

int FTPClient::lcd(const vector<string> &args) {

    cout << "LCD " << args[0] << endl;
    if (isExist(args[0]) == 1) {
        int n = chdir(args[0].c_str());
        cout << "Local directory now " << args[0] << endl;
        return n;
    } else {
        cout << "?Invalid command\n";
    }
    return 0;
}

int FTPClient::cd(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }
    int response_code;

    control.Send("CWD " + args[0] + "\r\n");
    response_code = receive_response_from_server();
    if (response_code == 250) {
        cout << "Directory successfully changed.\n";
        return response_code;
    } else {
        cout << "Failed to change directory.\n";
    }
    return 0;
}

int FTPClient::help(const vector<string> &args) {

    if (args.size() == 0) {
        cout
                << "Commands are:\n\tlogin\t\tls\t\t\tdir\n\tput \t\tget\t\t\tmput\n\tmget\t\tcd\t\t\tlcd\n\tdelete\t\tmdelete\t\tmkdir\n\trmdir\t\tpwd\t\t\tpassive\n\tquit\t\texit\n";
    } else {
        cout << "\t";
        if (args[0] == "login") {
            cout << "login       \tlogin to FTP Server\n";
        } else if ((args[0] == "ls") || (args[0] == "dir")) {
            cout << args[0] + "        \tlist contents of remote directory\n";
        } else if (args[0] == "put") {
            cout << "put       \tsend one file\n";
        } else if (args[0] == "get") {
            cout << "get       \treceive file\n";
        } else if (args[0] == "mput") {
            cout << "mput      \tsend multiple files\n";
        } else if (args[0] == "mget") {
            cout << "mget      \tget multiple files\n";
        } else if (args[0] == "cd") {
            cout << "cd        \tchange remote working directory\n";
        } else if (args[0] == "lcd") {
            cout << "lcd       \tchange local working directory\n";
        } else if (args[0] == "delete") {
            cout << "delete    \tdelete remote file\n";
        } else if (args[0] == "mdelete") {
            cout << "mdelete   \tdelete multiple files\n";
        } else if (args[0] == "mkdir") {
            cout << "mkdir     \tmake directory on the remote machine\n";
        } else if (args[0] == "rmdir") {
            cout << "rmdir     \tremove directory on the remote machine\n";
        } else if (args[0] == "pwd") {
            cout << "pwd       \tprint working directory on remote machine\n";
        } else if (args[0] == "passive") {
            cout << "passive   \tenter passive transfer mode\n";
        } else if ((args[0] == "quit") || (args[0] == "exit")) {
            cout << args[0] + "      \tterminate ftp session and exit\n";
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
    //Check number of args.
    if (args.empty() && args.size() > 2) {
        cout << "Usage\n";
        return -1;
    }
    int response_code;

    control.Send("DELE " + args[0] + "\r\n");
    response_code = receive_response_from_server();
    if (response_code == 250) {
        cout << "Delete operation successful\n";
        return response_code;
    } else if ((response_code == 450) || (response_code == 550)) {
        cout << "Delete operation failed\n";
    } else {
        //todo loi go lenh delete
    }
    return 0;
}

int FTPClient::mput(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    //Check number of args.
    if (args.empty()) {
        cout << "Usage: mput local_file_1 [local_file_2] [...]\n";
        return -1;
    };
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

    if (args.empty() && args.size() > 2) {
        return -1;
    }

    FILE *output;
    if (args.size() == 1) {
        output = fopen(args[0].c_str(), "wb");
    }
    else
        output = fopen(args[1].c_str(), "wb");

    void *data_connection;
    passive_mode ? data_connection = new TCPClient : data_connection = new TCPServer;

    if (!establish_data_connection(data_connection)) {
        cout << "Can not establish data connection\n";
        return -1;
    };

    control.Send("RETR " + args[0] + "\r\n");

    if (args.size() == 1)
        cout << "local: " << args[0] << " remote: " << args[0] << endl;
    else
        cout << "local: " << args[1] << " remote: " << args[0] << endl;

    int code = receive_response_from_server();
    if ((code == 150) || ((code == 125)))
    {

        char buff[BUFSIZE];
        clock_t t1, t2;
        int n, nSum = 0;

        t1 = clock();


        if (passive_mode) {
            TCPClient *data = (TCPClient *) data_connection;

            while (n = data->Receive(buff, BUFSIZE)) {
                nSum += n;
                fwrite(buff, 1, n, output);
            }

            t2 = clock();
            data->close_connection();
        } else {
            TCPServer *data = (TCPServer *) data_connection;

            while (n = data->Receive(buff, BUFSIZE)) {
                nSum += n;
                fwrite(buff, 1, n, output);
            }

            t2 = clock();
            data->close_connection();
        }

        fclose(output);

        code = receive_response_from_server();
        if ((code == 226) || (code == 250)) {

            float diff((float) t2 - (float) t1);
            diff /= CLOCKS_PER_SEC;
            float speed = (float) nSum / diff;
            cout << fixed;
            cout << nSum << " bytes received in " << setprecision(5) << diff << " secs" << " (" << speed / 1024
                 << "KB/s )\n";
            cout << code << " Transfer complete\n";

        } else if ((code == 425) || (code == 426) || (code == 451)) {
            cout << "Not connected\n";
            return -1;
        } else if ((code == 450) || (code == 550)) {
            cout << "Requested file action not taken\n";
            return -1;
        } else {
            cout << "false";
            cout << code;
        }
    } else{
        // TODO
    }

    return code;//receive_response_from_server();
}


int FTPClient::mdelete(const vector<string> &args)
{
    if (!control.isConnected()) {
        cout << "Not connected\n";
        return -1;
    }

    for (int i = 0; i < args.size(); i++) {
        vector<string> temp;
        temp.push_back(args[i]);
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

    //Check number of args.
    if (args.empty()) {
        cout << "Usage: mkdir folder_name\n";
        return -1;
    };
    int response_code;

    control.Send("MKD " + args[0] + "\r\n");

    response_code = receive_response_from_server();
    if (response_code == 257) {
        return response_code;
    } else {
        return -1;
    }
}

int FTPClient::rmdir(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    //Check number of args.
    if (args.empty()) {
        cout << "Usage: mkdir folder_name\n";
        return -1;
    };
    int response_code;

    control.Send("RMD " + args[0] + "\r\n");

    response_code = receive_response_from_server();

    if (response_code == 250) {
        return response_code;
    } else {
        return -1;
    }
}

int FTPClient::mget(const vector<string> &args) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    //Check number of args.
    if (args.empty()) {
        cout << "Usage\n";
        return -1;
    }
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
                TCPClient *data = (TCPClient *) data_connection;
                datalist = data->Receive();
                data->close_connection();

                delete[]data;
            } else {
                TCPServer *data = (TCPServer *) data_connection;
                datalist = data->Receive();
                data->close_connection();

                delete[]data;
            }
            code = receive_response_from_server();
            if ((code == 226) || (code == 250)) {
               cout << "Directory send ok.\n";
            } else if ((code == 425) || (code == 426) || (code == 451)) {
                cout << "Not connected\n";
            } else if (code == 450) {
                cout << "Requested file action not taken\n";
            } else {
                //TODO LOI GUI LENH LIST
            }

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

        } else {
            //TODO LOI GUI LENH mget
        }
    }
   else{
        for (int i = 0; i < args.size(); i++) {
            vector<string> temp;
            temp.push_back(args[i]);
            temp.push_back(args[i]);
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
        TCPClient *data = (TCPClient *) data_connection;

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
        TCPServer *data = (TCPServer *) data_connection;
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

