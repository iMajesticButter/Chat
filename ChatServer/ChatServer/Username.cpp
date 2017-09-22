#include "Username.h"


//add a user to the map
void Namelist::add(SOCKET* sock, std::string name) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	nameMap[*sock] = name;
	sockMap[name] = *sock;
}

//remove a user from the map
void Namelist::remove(SOCKET* sock) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	sockMap.erase(nameMap[*sock]);
	nameMap.erase(*sock);
}

//get username by socket
std::string Namelist::getName(SOCKET* sock) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	return nameMap[*sock];
}

//get socket by username
SOCKET Namelist::getSock(std::string name) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	return sockMap[name];
}

//see if a map contains a username
bool Namelist::contains(std::string name) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	return sockMap.find(name) != sockMap.end();
}

//get the number of units in the map
int Namelist::size() {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	return nameMap.size();
}

//change a username
void Namelist::changeName(SOCKET* sock, std::string newName) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	sockMap.erase(nameMap[*sock]);
	sockMap[newName] = *sock;
	nameMap[*sock] = newName;
}