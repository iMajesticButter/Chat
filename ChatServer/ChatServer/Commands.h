#pragma once
#include "RoomFuncs.h"
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <list>
#include <thread>
#include <fstream>

#pragma comment (lib, "ws2_32.lib")

class RoomList;
class Rooms;

//this file is for checking for commands and running them if they are found
//command functions return true if they find a command

namespace chatcmd {

	//a struct just to hold parsed messages for commands
	struct ParsedMSG {
		std::string username;
		std::string msg;
		std::string cmdmsg;
		bool command;
	};

	//parse a string into a username, message, and command message(the message but cut off at the first space)
	//args: char array
	ParsedMSG Parse(char[]);

	// --- /ping returns 'PONG' to the client it was sent from ---
	//args: SOCKET, ParsedMSG
	bool Ping(SOCKET, ParsedMSG);

	// --- /quit tells others in the room that the client is disconnecting and sends a msg to client to confirm ---
	//args: SOCKET, ParsedMSG, fd set, room name(string), SOCKET*(listening socket ptr)
	bool Quit(SOCKET, ParsedMSG, fd_set&, std::string, SOCKET*);

	// --- /silent does nothing... litarally nothing, it can be usefull, maybe ---
	//args: ParsedMSG
	bool Silent(ParsedMSG);

	// --- /echo is similar to /ping exept instead of returning 'Pong' it returns whatever the client sends after it ---
	// example: client sends '/echo hello' server responds 'hello'
	//args: SOCKET, ParsedMSG
	bool Echo(SOCKET, ParsedMSG);

	// --- /changename notifys others in the room that the client is changing their username and what their changing it to ---
	//args: SOCKET, ParsedMSG, fd set, room name(string), SOCKET*(listening socket ptr)
	bool Changename(SOCKET, ParsedMSG, fd_set&, std::string, SOCKET*);

	// --- /list lists all rooms ---
	//args: SOCKET, ParsedMSG, RoomList ptr
	bool List(SOCKET, ParsedMSG, RoomList*);

	// --- /createroom creates a room in a new thread if the entered name is availible, than auto-joins it, and notifys every on in the room that you are in that you are leaving ---
	//args: SOCKET, ParsedMSG, fd set, RoomList ptr, room name(string), SOCKET*(listening socket ptr)
	bool Createroom(SOCKET, ParsedMSG, fd_set&, RoomList*, std::string, SOCKET*);

	// --- /join joins the selected room and notifys others in the room that your joining, and notifys others in the room that your leaving ---
	//args: SOCKET, ParsedMSG, fd set, RoomList ptr, room name(string), SOCKET*(listening socket ptr)
	bool Join(SOCKET, ParsedMSG, fd_set&, RoomList*, std::string, SOCKET*);

	// --- /help sends the contents of a help text file to the client ---
	//args: SOCKET, ParsedMSG
	bool Help(SOCKET, ParsedMSG);
}