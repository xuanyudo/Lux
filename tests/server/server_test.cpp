#include "../../src/hub/server.h"

/*
void get_client_ip(const int c_fd, const string msg) {
	string response = "IP FOR " + to_string(c_fd) + " = " + client_ip_by_fd(c_fd);
	server_send(c_fd, response);
}
*/

int main(int argc, char* argv[]) {
	//server_commands["getip"] = get_client_ip;//Any Client->Server
	
	cout << "Starting server..." << endl;
	
	server_start();
	
	cout << "Input enabled, searching for connections..." << endl;
	
	bool has_connected = false;
	bool connected = false;
	
	while(true) {
		string s;
		cin >> s;
		
		connected = server_connections() != 0;
		
		if (connected) {
			if (!has_connected) {
				cout << "Connection found." << endl;
				has_connected = true;
			}
			server_send(4, s);//first connection is always fd=4
		}
		
		if (s.compare("exit") == 0) {
			break;
		}
	}
	
	cout << "Terminated." << endl;
	
	return 0;
}