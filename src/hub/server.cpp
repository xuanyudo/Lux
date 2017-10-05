#include "server.h"

int sockfd;
bool running = false;

cmd_map server_commands;
map<string, int> by_ip;

set<int> status_wait;//for responses to status requests
map<int, int> upd_wait;//<device fd, web client fd> for responses to update requests

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* accept_devices(void* client_addr);
void* read_client(void* client_fd_ptr);

//Client->Server
void client_exit(int client_fd, string message);
void client_test(int client_fd, string message);
void client_register(int client_fd, string message);
void client_connect(int client_fd, string message);
void client_status(int client_fd, string message);

//Web Client->Server
void client_unregister(int client_fd, string message);
void client_upd_req(int client_fd, string message);
void client_status_req(int client_fd, string message);
void client_status_ack(int client_fd, string message);

void server_start() {
	//Client->Server
	server_commands["exit"] = client_exit;
	server_commands["test"] = client_test;
	server_commands["register"] = client_register;
	server_commands["connect"] = client_connect;
	server_commands["status"] = client_status;
	
	//Web Client->Server
	server_commands["unregister"] = client_unregister;
	server_commands["update_request"] = client_upd_req;
	server_commands["status_request"] = client_status_req;
	server_commands["status_ack"] = client_status_ack;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd == -1) {
		cerr << "Failure to create server socket." << endl;
		return;
	}
	
	struct sockaddr_in hints;
	
	hints.sin_family = AF_INET;
	hints.sin_addr.s_addr = INADDR_ANY;
	hints.sin_port = htons(PORT);
	
	if (bind(sockfd, (struct sockaddr*) &hints, sizeof(hints)) == -1) {
		cerr << "Could not assign a name to the server socket." << endl;
		return;
	}
	
	if (listen(sockfd, LISTEN_QUEUE_SIZE) == -1) {
		cerr << "Failed to listen for connections on port " << PORT << "." << endl;
		return;
	}
	
	pthread_t acc_dev;//create accept devices thread so the server can still send data concurrently
	
	int err = pthread_create(&acc_dev, NULL, accept_devices, (void*) &hints);
	
	if (err != 0) {
		cerr << "Failed to create \"acc_dev\" thread." << endl;
		return;
	}
	
	/*err = pthread_join(acc_dev, NULL);
	
	if (err != 0) {
		cerr << "Failed to join \"acc_dev\" thread." << endl;
		//return;
	}*/
}

int server_connections() {
	pthread_mutex_lock(&mtx);
	int size = by_ip.size();
	pthread_mutex_unlock(&mtx);
	
	return size;
}

int client_fd_by_ip(string ip) {
	pthread_mutex_lock(&mtx);
	if (by_ip.count(ip) == 0) {
		return -1;
	}
	
	int fd = by_ip[ip];
	pthread_mutex_unlock(&mtx);
	
	return fd;
}

string client_ip_by_fd(int fd) {
	struct sockaddr_in addr;
	int a_size = sizeof(addr);
	
	if (getpeername(fd, (sockaddr*) &addr, (socklen_t*) &a_size)) {
		return "";//empty string
	}
	
	return inet_ntoa(addr.sin_addr);
}

void server_send(int c_fd, string msg) {
	if (!running) {
		cerr << "Server attempted to send to client " << c_fd << " while not running." << endl;
		return;
	}
	
	/*
	This allows for a message of MESSAGE_SIZE to be sent each time, meaning more bytes are sent,
	but there is no possibility that calling send() twice in rapid succession will concatenate messages in the buffer.
	*/
	
	char* a = new char[MESSAGE_SIZE + 1];
	memcpy(a, msg.c_str(), MESSAGE_SIZE);
	a[MESSAGE_SIZE] = 0;
	
	pthread_mutex_lock(&mtx);
	send(c_fd, a, MESSAGE_SIZE, 0);
	pthread_mutex_unlock(&mtx);
}

void* accept_devices(void* c_addr) {
	sockaddr_in hints = *(sockaddr_in*) c_addr;
	
	running = true;
	
	while(true) {
		int h_size = sizeof(hints);
		int c_fd = accept(sockfd, (struct sockaddr*) &hints, (socklen_t*) &h_size);
	
		if (c_fd == -1) {
			cerr << "Failed to establish a connection with a client." << endl;
			pthread_exit(0);
		}
	
		string ip = inet_ntoa(hints.sin_addr);
		
		pthread_mutex_lock(&mtx);
		cout << "Device " << c_fd << " connected from IP: " << ip << endl;
		
		string msg = "Connection established.";
		send(c_fd, msg.c_str(), msg.length(), 0);
		
		by_ip.insert(pair<string, int>(ip, c_fd));
		pthread_mutex_unlock(&mtx);
		
		pthread_t dev_rc;
		
		int err = pthread_create(&dev_rc, NULL, read_client, (void*) &c_fd);
	
		if (err != 0) {
			cerr << "Failed to create \"dev_rc\" thread for device at IP: " << ip << endl;
			pthread_exit(0);
		}
	
		/*err = pthread_join(dev_rc, NULL);
	
		if (err != 0) {
			cerr << "Failed to join \"dev_rc\" thread for device at IP: " << ip << endl;
			pthread_exit(0);
		}*/
	}
}

/*
list<string> split(string line, char delimiter){
    list<string> pieces;
	string save = string(line);
    int pos = 0;
    while ((pos = save.find(delimiter)) != string::npos) {
        pieces.push_back(save.substr(0, pos));
        save.erase(0, pos + 1);
    }
    return pieces;
}
*/

void* read_client(void* c_fd_p) {
	int c_fd = *(int*) c_fd_p;
	
	char msg[MESSAGE_SIZE];
	
	while(true) {
		memset(&msg, 0, MESSAGE_SIZE);//clear the buffer
		
		recv(c_fd, msg, MESSAGE_SIZE, 0);
		
		string s_msg(msg);
		
		if (s_msg.length() == 0) {//client is disconnected, the buffer is just being read constantly
			client_exit(c_fd, "");
		}
		
		Parser* p = new Parser(s_msg);
		
		string key = p->getCommand();//grab command type from JSON
		
		delete(p);
		
		if (server_commands.count(key) == 0) {
			pthread_mutex_lock(&mtx);
			cout << "[Device " << c_fd << "]" << s_msg << endl;
			pthread_mutex_unlock(&mtx);
		} else {
			pthread_mutex_lock(&mtx);
			cout << "Device " << c_fd << " ran command: " << key << endl;
			pthread_mutex_unlock(&mtx);
			
			server_commands.find(key)->second(c_fd, s_msg);//must be the last thing to happen as sometimes the thread is killed
		}
	}
}

//ACKNOWLEDGEMENTS:

void send_exit_ack(int c_fd) {
	Encode* e = new Encode();
	
	e->setCommand("exit_ack");
	
	//TODO what information is required?
	
	server_send(c_fd, e->stringfy());
	
	delete(e);
}

void send_status_ack(int c_fd) {
	Encode* e = new Encode();
	
	e->setCommand("status_ack");
	
	//TODO what information is required?
	
	server_send(c_fd, e->stringfy());
	
	delete(e);
}

void send_upd_ack(int c_fd) {
	Encode* e = new Encode();
	
	e->setCommand("update_ack");
	
	//TODO what information is required?
	
	server_send(c_fd, e->stringfy());
	
	delete(e);
}

//REQUESTS:

void send_status_req(int c_fd) {
	Encode* e = new Encode();
	
	e->setCommand("status_request");
	e->setUuid(c_fd);
	
	//TODO what information is required?
	
	server_send(c_fd, e->stringfy());
	
	delete(e);
}

//RESPONSES:

void client_exit(int c_fd, string msg) {
	string ip = client_ip_by_fd(c_fd);
	
	if (ip.compare("") == 0) {
		cerr << "Attempted to exit invalid device " << c_fd << "." << endl;
		return;
	}
	
	by_ip.erase(ip);//if ip exists in the map
	
	send_exit_ack(c_fd);
	
	cout << "Device " << c_fd << " (" << ip << ") disconnected." << endl;
	pthread_exit(0);
}

void client_test(int c_fd, string msg) {
	string succ = "SUCCESS";
	server_send(c_fd, succ);
}

void client_register(int c_fd, string msg) {
	Parser* p = new Parser(msg);
	
	string d_name = p->getDeviceName();
	
	if (!isValidName(d_name)) {
		cerr << "Invalid device name: " << d_name << endl;
		delete(p);
		return;
	}
	
	Device* d = new Device(p->getUUID(), p->getIP(), d_name);
	
	//TODO any information sent in this message must be added to the device here
	
	string g_name = p->getGroupName();
	
	if (!isValidGroupName(g_name)) {
		cerr << "Invalid group name: " << g_name << endl;
		delete(p);
		delete(d);
		return;
	}
	
	DeviceGroup* g;
	
	if (grps_n.count(g_name) == 0) {//no group by that name exists
		g = new DeviceGroup(g_name);
	} else {
		DeviceGroup g_perm = byGroupName(g_name);
		g = &g_perm;
	}
	
	g->addDevice(d);
	
	send_status_req(c_fd);
	
	delete(d);
}

void client_connect(int c_fd, string msg) {
	Parser* p = new Parser(msg);
	
	string d_name = p->getDeviceName();
	
	if (!isValidName(d_name)) {
		cerr << "Invalid device name: " << d_name << endl;
		delete(p);
		return;
	}
	
	Device d = byID(p->getUUID());
	
	//TODO the client should automatically connect due to accept_devices
	
	send_status_req(c_fd);
	
	delete(p);
}

void client_status(int c_fd, string msg) {
	Parser* p = new Parser(msg);
	
	int id = p->getUUID();
	
	//TODO change group if it's different
	
	Device d = byID(id);
	
	int wc_fd = -1;
	bool rtrv = upd_wait.count(id) > 0;
	
	if (rtrv) {
		wc_fd = upd_wait[id];
		upd_wait.erase(id);
	}
	
	//TODO set necessary information in the Device class
	
	send_status_ack(c_fd);
	
	if (rtrv) {
		send_upd_ack(wc_fd);
	}
	
	delete(p);
}

void client_unregister(int c_fd, string msg) {
	Parser* p = new Parser(msg);
	
	string d_name = p->getDeviceName();
	
	if (!isValidName(d_name)) {
		cerr << "Invalid device name: " << d_name << endl;
		return;
	}
	
	Device* d = new Device(p->getUUID(), p->getIP(), d_name);
	
	string g_name = p->getGroupName();
	
	if (!isValidGroupName(g_name)) {
		cerr << "Invalid group name: " << g_name << endl;
		delete(d);
		return;
	}
	
	if (grps_n.count(g_name) == 0) {//no group by that name exists
		cerr << "Attempted to unregister a device (" << c_fd << ") from an invalid group." << endl;
		delete(d);
		return;
	}
	
	DeviceGroup g = byGroupName(g_name);
	
	g.removeDevice(d);
	
	delete(d);
}

void client_upd_req(int c_fd, string msg) {
	Parser* p = new Parser(msg);
	
	string d_name = p->getDeviceName();
	
	if (!isValidName(d_name)) {
		cerr << "Invalid device name: " << d_name << endl;
		return;
	}
	
	int id = p->getUUID();
	
	Device* d = new Device(id, p->getIP(), d_name);
	
	Encode* e = new Encode();
	
	e->setCommand("update");
	
	server_send(id, e->stringfy());
	
	upd_wait.insert(pair<int, int>(id, c_fd));
	
	delete(p);
	delete(d);
	delete(e);
}

void client_status_req(int c_fd, string msg) {
	pid_t pid = fork();
	
	if (pid == 0) {//child
		
		for (map<string, DeviceGroup*>::iterator it = grps_n.begin(); it != grps_n.end(); ++it) {
			string g_name = it->first;
			DeviceGroup* g = it->second;
		
			list<Device*> devs = g->getDevices();
		
			//send a status command for each device
			for (list<Device*>::iterator dit = devs.begin(); dit != devs.end(); ++dit) {
				Device* d = *dit;
			
				Encode* e = new Encode();
			
				e->setCommand("status");
				e->setGroupName(g_name);
				e->setDeviceName(d->getName());
				e->setIP(d->getIP());
				e->setLight_L(d->getLightLevel());
				e->setUuid(d->getID());
			
				server_send(c_fd, e->stringfy());
				
				//wait for status_ack
				pthread_mutex_lock(&mtx);
				status_wait.insert(c_fd);
				pthread_mutex_unlock(&mtx);
			
			    bool rcvd = false;
				
				while(!rcvd) {
					pthread_mutex_lock(&mtx);
				    rcvd = status_wait.count(c_fd) == 0;//status received
				    pthread_mutex_unlock(&mtx);
				}
			}
		}
		
	}//parent goes back to reading the client
}

void client_status_ack(int c_fd, string msg) {
	pthread_mutex_lock(&mtx);
	if (status_wait.count(c_fd) > 0) {
		status_wait.erase(c_fd);
	} else {
		cerr << "Client (" << c_fd << ") acknowledged nonexisting status command." << endl;
	}
	pthread_mutex_unlock(&mtx);
}