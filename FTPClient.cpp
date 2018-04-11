#include "FTPClient.h"

FTPClient::FTPClient()
{
    current_path = getCurrentPath();
	isRunning = true;
}

int FTPClient::setCurrentPath(const string &path)
{
	return chdir(path.c_str());
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
	if (verbose) cout << response_str;

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

	string response_str;
	int response_code;

	control.Send("QUIT\r\n");

    response_str = control.Receive();
	if (verbose) cout << response_str << endl;

	/*QUIT
         221			Closing connection
         500			Syntax error
	*/
	response_code = stoi(response_str);
	if (response_code == 221) {
        control.close_connection();
		cout << "Connection closed\n";
	} else cout << "Syntax error\n";

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

    if (!passive_mode) {
        TCPServer data;
        string response_str;
        int response_code;

        data.wait_for_connection();
        sleep(1);

        //Send port information to server
        control.Send("PORT " + control.get_client_address() + "," + data.get_server_port() + "\r\n");
        response_str = control.Receive();
        if (verbose) cout << response_str;

        response_code = stoi(response_str);
        if (response_code != 200) {
            //TODO LOI GUI LENH PORT
        } else {
            control.Send("STOR " + remoteFileName + "\r\n");
            response_str = control.Receive();
            if (verbose) cout << response_str;
            response_code = stoi(response_str);
            if (response_code != 150) {
                //TODO LOI GUI LENH STORE
            } else {
                while (!feof(input)) {
                    char buff[BUFSIZE];
                    int nbytes = fread(buff, 1, BUFSIZE, input);

                    data.Send(buff, nbytes);
                }
            }

            data.close_connection();
            response_str = control.Receive();
            if (verbose) cout << response_str;
            response_code = stoi(response_str);
        }

    } else {
        TCPClient data;
        string response_str;
        int response_code;

        //Enter passive mode
        control.Send("PASV\r\n");
        response_str = control.Receive();
        if (verbose) cout << response_str;
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

            data.setup(address, port);

            control.Send("STOR " + remoteFileName + "\r\n");
            response_str = control.Receive();
            if (verbose) cout << response_str;
            response_code = stoi(response_str);
            if (response_code != 150) {
                //TODO LOI GUI LENH PORT
            } else {
                while (!feof(input)) {
                    char buff[BUFSIZE];
                    int nbytes = fread(buff, 1, BUFSIZE, input);

                    data.Send(buff, nbytes);
                }
            }

            data.close_connection();
        }
    }

    return 0;
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

    if (relativePath[0] == '/') return "/" + fileName;
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
//
//string FTPClient::getParentPath(int nLevels) {
//    string parentDenotes;
//    string realPath;
//
//    for (int i = 0; i < nLevels; i++) parentDenotes += "../";
//
//    char *temp = realpath(parentDenotes.c_str(), nullptr);
//
//    if (temp != nullptr) {
//        realPath.assign(temp);
//        free(temp);
//    }
//
//    return realPath;
//}

inline bool FTPClient::isExist(const string &fileName) {
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

int FTPClient::login(const vector<string> &arg) {
    if (!control.isConnected()) {
        cout << "Not connected.\n";
        return -1;
    }

    string response_str;
    int response_code;

    control.Send("USER " + arg[0] + "\r\n");
    response_str = control.Receive();
    if (verbose) cout << response_str;

    response_code = stoi(response_str);
    if (response_code != 331) {
        //TODO LOI GUI LENH USER
    } else {
        control.Send("PASS " + arg[1] +"\r\n");

        response_str = control.Receive();
        if (verbose) cout << response_str;

        response_code = stoi(response_str);
        if (response_code == 230) {
            cout<<"Login successful\n";
        }
        else if (response_code == 530)
        {
            cout<<"Login incorrect\n";
            cout<<"Login failed\n";
        } else{
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
    if (verbose) cout << response_str;

    response_code = stoi(response_str);
    if (response_code == 257) {
        cout << response_str;
    } else {
        //TODO LOI GUI LENH PWD
    }

    return response_code;
}
