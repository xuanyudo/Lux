#include "../../src/hub/server.h"

void get_client_ip(const int c_fd, const string msg) {
	string response = "IP FOR " + to_string(c_fd) + " = " + client_ip_by_fd(c_fd);
	server_send(c_fd, response);
}

int main(int argc, char* argv[]) {
	server_commands["getip"] = get_client_ip;//Any Client->Server
	
	cout << "Starting server..." << endl;
	
	server_start();
	
	cout << "Started. Waiting for connection..." << endl;
	
	while(true) {
		if (server_connections() == 0) {
			continue;
		}
		
		cout << "Connection found." << endl;
		break;
	}
	
	cout << "Reading input..." << endl;
	
	while(true) {
		string s;
		cin >> s;
		
		server_send(4, s);//first connection is always fd=4
	}
	
	cout << "Terminated." << endl;
	
	return 0;
}