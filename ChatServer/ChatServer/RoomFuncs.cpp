#include "RoomFuncs.h"


//the constructor for creating the list of rooms, makes sure the list is empty
RoomList::RoomList() {
	Rooms.clear();
}

//Reads from the list of rooms
Room& RoomList::Read(int index) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	//return the index'th element in the list
	std::list<Room>::iterator it = Rooms.begin();
	std::advance(it, index);
	return *it;
}

//write to the list of rooms
void RoomList::Write(Room* toWrite) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	//write toWrite to the front of the list
	Rooms.push_front(*toWrite);
}

//remove an element from the list
void RoomList::Remove(Room* r) {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	//remove the room from the list
	Rooms.remove(*r);
}

//get the size of the list
int RoomList::GetSize() {
	//lock the mutex so other threads will not mess thing up...
	std::lock_guard<std::mutex> locker(_mu);
	//return the size of the list
	return Rooms.size();
}