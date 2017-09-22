#pragma once
#include "Username.h"
#include <list>
#include <WS2tcpip.h>
#include <io.h>
#include <mutex>

#pragma comment (lib, "ws2_32.lib")

//declare classes
class RoomList;

//a room is a group of clients that will send and receve messages among each-other
class Room {

public:
	//stuff so that we can compare rooms
	bool operator == (const Room& r) const { return roomName == r.roomName; }
	bool operator != (const Room& r) const { return !operator==(r); }

	//the fd set that holds all sockets connected to the room
	fd_set roomSet;
	//a pointer to the listening socket 4 functions that need it
	SOCKET* listening;
	//the name of the room
	std::string roomName;
	//a line that you have to wait in before you can connect to the room
	std::list<SOCKET> connectionQ;
	//a mutex for locking other threads out of stuff
	std::mutex _mu;

	//the constructior for the room
	Room(std::string);

	//a copy constructior for the room cuz the implicit one tryed to copy athe mutex...
	Room(const Room&);

	//the thing that the rooms thread will be running on, where messages get processed, and sockets the line can be accepted into the room
	void Process(RoomList&, Namelist& names);

	//add's a socket into line for waiting to connect to the room
	void Add(SOCKET&, fd_set&);
};