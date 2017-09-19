#pragma once
#include "Room.h"
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <list>
#include <iterator>
#include <thread>
#include <mutex>

#pragma comment (lib, "ws2_32.lib")

//declare classes
class Room;

//the list of rooms protected in a class
class RoomList {
private:
	//the list of rooms
	std::list <Room> Rooms;
	//the mutex that protects the list of rooms
	std::mutex _mu;

public:
	//the constructor for creating the list of rooms, makes sure the list is empty
	RoomList();

	//Reads from the list of rooms
	Room& Read(int);

	//write to the list of rooms
	void Write(Room*);

	//remove an element from the list
	void Remove(Room*);

	//get the size of the list
	int GetSize();

};