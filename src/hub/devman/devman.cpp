#include "devman.h"

#define MAX_NAME_LENGTH 20

//BEGIN EXTERNAL DEFINITIONS
map<string, DeviceGroup*> grps_n;
map<string, Device*> devs_id;
map<string, Device*> devs_ip;
map<string, Device*> devs_n;
//END EXTERNAL DEFINITIONS

char delim = '|';

//BEGIN Device
Device::Device(string uuid, string ip, string name) {
	this->uuid = uuid;
	this->ip = ip;
	this->name = name;
	
	f_vers = "UNKNOWN";
	h_vers = "UNKNOWN";
	
	devs_id.insert(pair<string, Device*>(this->uuid, this));
	devs_ip.insert(pair<string, Device*>(this->ip, this));
	devs_n.insert(pair<string, Device*>(this->name, this));
}

Device::~Device(){}

string Device::getUUID() const {
	return uuid;
}

string Device::getIP() const {
	return ip;
}

string Device::getName() const {
	return name;
}

int Device::getLightLevel() const {
	return level;
}

void Device::setLightLevel(int level) {
	this->level = level;
}

string Device::firmware_version() const {
	return f_vers;
}

string Device::hardware_version() const {
	return h_vers;
}

void Device::set_f_vers(string f_vers) {
	this->f_vers = f_vers;
}

void Device::set_h_vers(string h_vers) {
	this->h_vers = h_vers;
}

string Device::toString() {
	return uuid + delim + ip + delim + name + delim + to_string(level) + delim + f_vers + delim + h_vers;
}

bool Device::operator ==(const Device dev) const {
	return dev.getUUID().compare(this->uuid) == 0 && dev.getIP() == this->ip && dev.getName().compare(this->name) == 0;
}
//END Device

//BEGIN DeviceGroup
DeviceGroup::DeviceGroup(string name) {
	this->name = name;
	
	devices = 0;
	
	grps_n.insert(pair<string, DeviceGroup*>(this->name, this));
}

DeviceGroup::~DeviceGroup(){}

void DeviceGroup::addDevice(Device* l) {
	g_devs.push_back(l);
	devices++;
}

bool DeviceGroup::removeDevice(Device* l) {
	if (*find(g_devs.begin(), g_devs.end(), l) == *g_devs.end()) {//no device found
		return false;
	}
	g_devs.remove(l);
	
	delete l;
	
	devices--;
	return true;
}

int DeviceGroup::size() {
	return devices;
}

list<Device*> DeviceGroup::getDevices() {
	return g_devs;
}

string DeviceGroup::getName() {
	return name;
}
//END DeviceGroup

Device byUUID(string uuid) {
	if (devs_id.count(uuid) == 0) {//no device found
		return Device("NULL", "NULL", "NULL");//TODO throw custom exception?
	}
	return *devs_id[uuid];
}

Device byIP(string ip) {
	if (devs_ip.count(ip) == 0) {//no device found
		return Device("NULL", "NULL", "NULL");
	}
	return *devs_ip[ip];
}

Device byName(string name) {
	if (devs_n.count(name) == 0) {//no device found
		return Device("NULL", "NULL", "NULL");
	}
	return *devs_n[name];
}

//BEGIN UTILITY FUNCTIONS
bool isProperLength(string name) {
	return name.length() <= MAX_NAME_LENGTH;
}

bool legalChars(string name, string legal) {
	for (int i = 0; i < legal.length(); i++) {
		if (name.find(legal[i]) != -1) {//string::npos = -1, return value of string::find() if no matches
			continue;
		}
		return false;//contains invalid character
	}
	return true;
}
//END UTILITY FUNCTIONS

bool isValidName(string name) {
	string legal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 ";
	return isProperLength(name) && legalChars(name, legal) && name.compare("NULL") != 0 && byName(name).getUUID().compare("NULL") != 0;//valid name
}

bool isValidGroupName(string name) {
	string legal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 ";
	return isProperLength(name) && legalChars(name, legal) && name.compare("NULL") != 0 && byGroupName(name).getName().compare("NULL") != 0;
	//valid name
}

bool isValidVersion(string vers) {
	string legal = ".1234567890";
	return isProperLength(vers) && legalChars(vers, legal);
}

DeviceGroup byGroupName(string name) {
	if (grps_n.count(name) == 0) {//no group found
		return DeviceGroup("NULL");
	}
	return *grps_n[name];
}

void updateFile(string filename) {
	ofstream file;
	
	file.open(filename.c_str(), ios::trunc);//creates file/*overwrites existing*
	
	list<string> used;
	
	for (map<string, DeviceGroup*>::iterator it = grps_n.begin(); it != grps_n.end(); ++it) {
		string grpn = it->first;
		DeviceGroup* grp = it->second;
		
		if (find(used.begin(), used.end(), grpn) == used.end()) {
			file << grpn << " (" << grp->size() << "):" << endl;//print group name and # devices if not printed
			used.push_back(grpn);
		}
		
		list<Device*> devs = grp->getDevices();
		
		for (list<Device*>::iterator dit = devs.begin(); dit != devs.end(); ++dit) {//print all devices in the group
			Device* dev = *dit;
			
			file << "\t" << dev->toString() << endl;
			//    ID : IP : NAME : FV : HV
		}
	}
	
	file.close();
}

//BEGIN UTILITY FUNCTION
string trim(string line) {
	string trimmed = line;
	while(trimmed[0] == ' ' || trimmed[0] == '\t') {
		trimmed = trimmed.substr(1);
	}
	while(trimmed[trimmed.length() - 1] == ' '|| trimmed[trimmed.length() - 1] == '\t') {
		trimmed = trimmed.substr(0, trimmed.length() - 1);
	}
	return trimmed;
}

vector<string> split(string line, char delimiter){
    vector<string> list;
	string save = string(line);
    int pos = 0;
    while ((pos = save.find(delimiter)) != string::npos) {
        list.push_back(save.substr(0, pos));
        save.erase(0, pos + 1);
    }
    return list;
}
//END UTILITY FUNCTION

bool loadFile(string filename) {
	ifstream file;
	
	file.open(filename.c_str());
	
	string line;
	
	DeviceGroup* grp;
	
	string uuid;
	int level;
	string ip, name, f_vers, h_vers;
	
	while(getline(file, line)) {
		if (line[0] == '\t') {//device
			if (grp == NULL) {
				return false;//INCORRECT FORMATTING, DEVICE/S OUTSIDE OF GROUP
			}
			
			vector<string> data = split(line, delim);
			
			uuid = trim(data[0]);
			ip = trim(data[1]);
			name = trim(data[2]);
			level = atoi(trim(data[3]).c_str());
			f_vers = trim(data[4]);
			h_vers = trim(line.substr(line.rfind(delim) + 1));
			
			Device* dev = new Device(uuid, ip, name);
			
			dev->setLightLevel(level);
			dev->set_f_vers(f_vers);
			dev->set_h_vers(h_vers);
			
			grp->addDevice(dev);
		} else {//devicegroup
			grp = new DeviceGroup(trim(line.substr(0, line.find("("))));
		}
	}
}