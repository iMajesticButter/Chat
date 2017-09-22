#pragma once
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

class Namelist {

private:
	//the map for acessing name by socket
	std::map<SOCKET, std::string> nameMap;
	//the map for acessing socket by name
	std::map<std::string, SOCKET> sockMap;
	//mutex for locking map
	std::mutex _mu;
public:
	//add a user to the map
	void add(SOCKET*, std::string);
	//remove a user from the map
	void remove(SOCKET*);
	//get username by socket
	std::string getName(SOCKET*);
	//get socket by username
	SOCKET getSock(std::string);
	//see if a map contains a username
	bool contains(std::string);
	//get the number of units in the map
	int size();
	//change a username
	void changeName(SOCKET*, std::string);
};