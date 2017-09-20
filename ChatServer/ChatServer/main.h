#pragma once
#include "Commands.h"
#include "Room.h"
#include "RoomFuncs.h"
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <fstream>

#pragma comment (lib, "ws2_32.lib")
//declare classes
class Room;
class RoomList;

//functions
//process all incoming messages (relay to other clients, parse commands, etc...)
void ProcessIncomingMSG(SOCKET, fd_set&, SOCKET, std::string, RoomList&);
//process clients in the master set
void ProcessMaster(SOCKET, fd_set&, SOCKET, RoomList&);
//send message to all but one client and not to the listening socket
void SendMSG(SOCKET, SOCKET, std::string, fd_set);
//parse a massage and run any commands entered sutch as /join, of /createroom etc...
chatcmd::ParsedMSG ParseAndCommands(SOCKET, SOCKET, fd_set&, char[], RoomList&, std::string);
//process everyone in the master set
void Master(fd_set&, SOCKET, RoomList&);