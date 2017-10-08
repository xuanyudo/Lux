#include "server.h"

int sockfd;
bool running = false;

cmd_map server_commands;

map<int, string> fds;//<fd, uuid>
map<string, int> uuids;//<uuid, fd>

map<string, int> by_ip;

set<int> status_wait;//<web client fd> for responses to status requests
set<string> conn_devs;//<serial #>

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

void server_start() {
	//Client->Server
	server_commands[DISCONNECT_REQUEST] = client_exit;
	server_commands[DISCONNECT] = client_exit;
	server_commands[FORCE_DISCONNECT] = client_exit;//TODO these should be different
	
	server_commands[TEST] = client_test;
	server_commands[REGISTER] = client_register;
	server_commands[CONNECT] = client_connect;
	server_commands[STATUS] = client_status;
	
	//Web Client->Server
	server_commands[UNREGISTER] = client_unregister;
	server_commands[UPDATE_REQUEST] = client_upd_req;
	server_commands[STATUS_REQUEST] = client_status_req;
	
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
}

//BEGIN GETTERS

int server_connections() {
	pthread_mutex_lock(&mtx);
	int size = fds.size();
	pthread_mutex_unlock(&mtx);
	
	return size;
}

int client_fd_by_uuid(string uuid) {
	pthread_mutex_lock(&mtx);
	if (uuids.count(uuid) == 0) {
		return -1;
	}
	
	int fd = uuids[uuid];
	pthread_mutex_unlock(&mtx);
	
	return fd;
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

string client_uuid_by_fd(int fd) {
	pthread_mutex_lock(&mtx);
	if (fds.count(fd) == 0) {
		return "";
	}
	
	string uuid = fds[fd];
	pthread_mutex_unlock(&mtx);
	
	return uuid;
}

string client_ip_by_fd(int fd) {
	struct sockaddr_in addr;
	int a_size = sizeof(addr);
	
	if (getpeername(fd, (sockaddr*) &addr, (socklen_t*) &a_size)) {
		return "";//empty string
	}
	
	return inet_ntoa(addr.sin_addr);
}

//END GETTERS

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
	}
}

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
		
		Json* json = new Json(s_msg);
		
		int cmd = json->cmd;//grab command type from JSON
		
		delete(json);
		
		pthread_mutex_lock(&mtx);
		cout << "Device " << c_fd << " ran command: " << cmd << endl;
		pthread_mutex_unlock(&mtx);
			
		server_commands.find(cmd)->second(c_fd, s_msg);//must be the last thing to happen as sometimes the thread is killed
	}
}

//REQUESTS:

void send_status_req(int c_fd) {
	Json* json = new Json(STATUS_REQUEST, client_uuid_by_fd(c_fd), REG_KEY);
	
	server_send(c_fd, json->jsonify());
	
	delete(json);
}

//RESPONSES:

void client_exit(int c_fd, string msg) {
	string ip = client_ip_by_fd(c_fd);
	
	if (ip.compare("") == 0) {
		cerr << "Attempted to exit invalid device " << c_fd << "." << endl;
		return;
	}
	
	uuids.erase(client_uuid_by_fd(c_fd));
	fds.erase(c_fd);
	by_ip.erase(ip);//if ip exists in the map
	
	cout << "Device " << c_fd << " (" << ip << ") disconnected." << endl;
	pthread_exit(0);
}

void client_test(int c_fd, string msg) {
	string succ = "SUCCESS";
	server_send(c_fd, succ);
}

void client_register(int c_fd, string msg) {
	Json* json = new Json(msg);
	
	//TODO get reg_key, compare to REG_KEY in server.h, if equal, add to conn_devs
	
	string d_name = json->data["name"];
	
	if (!isValidName(d_name)) {
		cerr << "Invalid device name: " << d_name << endl;
		delete(json);
		return;
	}
	
	Device* d = new Device(json->uuid, client_ip_by_fd(c_fd), d_name);
	
	//TODO any information sent in this message must be added to the device here
	
	string g_name = "all";//TODO every device will be stored in the all group for now
	
	if (!isValidGroupName(g_name)) {
		cerr << "Invalid group name: " << g_name << endl;
		delete(json);
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
	Json* json = new Json(msg);
	
	string d_name = json->data["name"];
	
	if (!isValidName(d_name)) {
		cerr << "Invalid device name: " << d_name << endl;
		delete(json);
		return;
	}
	
	Device d = byUUID(json->uuid);
	
	//TODO the client should automatically connect due to accept_devices
	
	send_status_req(c_fd);
	
	delete(json);
}

void client_status(int c_fd, string msg) {
	Json* json = new Json(msg);
	
	string uuid = json->uuid;
	
	//TODO change group if it's different
	
	Device d = byUUID(uuid);
	
	//TODO set necessary information in the Device class
	
	delete(json);
}

void client_unregister(int c_fd, string msg) {
	Json* json = new Json(msg);
	
	string d_name = json->data["name"];
	
	if (!isValidName(d_name)) {
		cerr << "Invalid device name: " << d_name << endl;
		return;
	}
	
	Device* d = new Device(json->uuid, client_ip_by_fd(c_fd), d_name);
	
	string g_name = "all";//TODO the only group currently is all
	
	if (!isValidGroupName(g_name)) {
		cerr << "Invalid group name: " << g_name << endl;
		delete(json);
		delete(d);
		return;
	}
	
	if (grps_n.count(g_name) == 0) {//no group by that name exists
		cerr << "Attempted to unregister a device (" << c_fd << ") from an invalid group." << endl;
		delete(json);
		delete(d);
		return;
	}
	
	DeviceGroup g = byGroupName(g_name);
	
	g.removeDevice(d);
	
	delete(json);
	delete(d);
}

void client_upd_req(int c_fd, string msg) {
	Json* rcv_json = new Json(msg);
	
	string d_name = rcv_json->data["name"];
	
	if (!isValidName(d_name)) {
		cerr << "Invalid device name: " << d_name << endl;
		return;
	}
	
	string uuid = rcv_json->uuid;
	
	Device* d = new Device(uuid, client_ip_by_fd(c_fd), d_name);
	
	Json* snd_json = new Json(UPDATE, uuid, REG_KEY);
	
	server_send(client_fd_by_uuid(uuid), snd_json->jsonify());
	
	delete(rcv_json);
	delete(d);
	delete(snd_json);
}

void client_status_req(int c_fd, string msg) {
	for (map<string, DeviceGroup*>::iterator it = grps_n.begin(); it != grps_n.end(); ++it) {
		string g_name = it->first;
		DeviceGroup* g = it->second;
		
		list<Device*> devs = g->getDevices();
		
		//send a status command for each device
		for (list<Device*>::iterator dit = devs.begin(); dit != devs.end(); ++dit) {
			Device* d = *dit;
			
			Json* json = new Json(STATUS, d->getUUID(), REG_KEY);
				
			json->data["name"] = d->getName();
			json->data["group_name"] = g_name;
			json->data["ip"] = d->getIP();
			json->data["level"] = d->getLightLevel();
			//TODO add f_vers, h_vers
			
			server_send(c_fd, json->jsonify());
				
			delete(json);
		}
	}
}