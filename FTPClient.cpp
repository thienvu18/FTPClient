#include "FTPClient.h"

FTPClient::FTPClient()
{
	current_path = getexepath();
	isRunning = true;
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

int FTPClient::open(const vector<string> &arg) {
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
	if (arg.empty() && arg.size() > 2) goto usage;

	if (regex_match(arg[0], ip) || regex_match(arg[0], url)) serverName = arg[0];
	else goto invalid_args;

	if (arg.size() == 2) {
		if (regex_match(arg[1], number)) port = stoi(arg[1]);
		else goto bad_port;
	}

	if (!control.setup(serverName, port)) {
		cout << "Can not connect to server\n";
		return -1;
	}

	response_str = control.receive();
	if (verbose) cout << response_str;

	if (response_str[0] < '1' || response_str[0] > '5') {
		cout << "Connection established, but this is not a ftp connection\nClose connection\n";
		control.exit();
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
		response_str = control.receive();    //Wait until receive ok meassage
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
	string response_str;
	int response_code;

	control.Send("QUIT\r\n");

	response_str = control.receive();
	if (verbose) cout << response_str << endl;

	/*QUIT
         221			Closing connection
         500			Syntax error
	*/
	response_code = stoi(response_str);
	if (response_code == 221) {
		control.exit();
		cout << "Connection closed\n";
	} else cout << "Syntax error\n";

	return response_code;
}

FTPClient::~FTPClient() {
	control.exit();
}