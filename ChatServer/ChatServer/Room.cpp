#include "Room.h"
#include "main.h"
#include "RoomFuncs.h"
#include <iterator>

//the constructior for the room
Room::Room(std::string InroomName) {
	//initialize things
	roomName = InroomName;
	FD_ZERO(&roomSet);
	FD_ZERO(&connectionQ);
}

//a copy constructior for the room cuz the implicit one tryed to copy athe mutex...
Room::Room(const Room& other) {
	//copy EVERYTHING (exept the mutex) from the original room to the new one
	this->roomName = other.roomName;
	this->listening = other.listening;
	this->roomSet = other.roomSet;
	this->connectionQ = other.connectionQ;
}

//the thing that the rooms thread will be running on, where messages get processed, and sockets the line can be accepted into the room
void Room::Process(RoomList& Rooms, Namelist& names, fd_set& master) {
    std::cout << "Thread started!" << std::endl;

	//make sure the first person joins the room before you decide to close the room!
	while (roomSet.fd_count < 1) {
		std::cout << "waiting..." << std::endl;
		{
			//lock the mutex so the main thread will not mess thing up...
			std::lock_guard<std::mutex> locker(_mu);
			//iterate thru all sockets that are in line
			std::list <SOCKET> ::iterator it;
			for (it = connectionQ.begin(); it != connectionQ.end(); ++it) {
				//add the socket to the room
				FD_SET(*it, &roomSet);
			}
			//clear the line so we dont end up with clones...
			connectionQ.clear();
			//mutex unlocks when it exits scope
		}
	}
	//the rooms message processing loop (ends when no-one is connected to the room anymore)
	while (true){
		std::cout << "Thread looped!" << std::endl;
		// see if I need to add newcomers to roomSet
		{
			//lock the mutex so the main thread will not mess thing up...
			std::lock_guard<std::mutex> locker(_mu);
			//iterate thru all sockets that are in line
			std::list <SOCKET> ::iterator it;
			for (it = connectionQ.begin(); it != connectionQ.end(); ++it) {
				//add the socket to the room
				FD_SET(*it, &roomSet);
			}
			//clear the line so we dont end up with clones...
			connectionQ.clear();
			//if there is no one in the room even after adding anyone in the cue, break the loop and delete the room
			if (roomSet.fd_count <= 0)
				break;
			//mutex unlocks when it exits scope
		}
		//copy the rooms set of connected sockets so it won't get messed with by select()
		fd_set setCopy = roomSet;
		//set up a 3 second timeout for select so that we can check for new connections even when no one is talking
		timeval tv;
		tv.tv_usec = 0;
		tv.tv_sec = 3;
		//call select() to wait until we receve a message on any of the sockets in the room or until the 3 second timer runs out
		int socketCount = select(0, &setCopy, nullptr, nullptr, &tv);
		//if select returned somthing other than 0 we receved a message
		if (socketCount != 0) {
			//print the number of sockets connected to the room
			std::cout << "connected count: " << roomSet.fd_count << std::endl;
			//print the number of sockets we have receved messages from
			std::cout << "socket count   : " << socketCount << std::endl;
			//loop thru the sockets we have receved messages from
			for (int i = 0; i < socketCount; ++i) {
				//sock is the socket currently being processed in this iteration of the for loop
				SOCKET sock = setCopy.fd_array[i];
				//process incoming message on sock (relay to other clients, parse commands, etc...)
				ProcessIncomingMSG(sock, roomSet, sock, roomName, Rooms, &names, master);
			}
		}
		//if select returned a 0 the timeout happened cuz no one said anything for 3 seconds
		else {
			std::cout << "Timeout on    : " << roomName << std::endl;
		}
	}
	//close down the room
	Rooms.Remove(this);
}

//add's a socket into line for waiting to connect to the room
void Room::Add(SOCKET& sockToAdd, fd_set& oldSet) {

	//send welcome to the room message to the connecting socket
	std::string str = "SERVER: Welcome to the room: '" + roomName + "'";
	std::cout << str << std::endl;
	send(sockToAdd, str.c_str(), str.length() + 1, 0);

	//add sockets to coonectionQ (the line that they have to wait in)
	std::lock_guard<std::mutex> locker(_mu);
	FD_CLR(sockToAdd, &oldSet);
	connectionQ.push_back(sockToAdd);

}


