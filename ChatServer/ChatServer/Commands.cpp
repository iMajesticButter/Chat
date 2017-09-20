#include "Commands.h"
#include "main.h"

namespace chatcmd {

	//parse a string into a username, message, and command message(the message but cut off at the first space)
	ParsedMSG Parse(char buf[]) {
		ParsedMSG parsed;
		//parse
		std::string s = buf;											//convert buf to string
		std::string del = "> ";											//the "> " after every username
		parsed.username = s.substr(0, s.find(del));						//remove message and "> " leaving just the username
		parsed.msg = s.substr(s.find(del) + del.length(), s.length());	//remove username and "> " leaving just the message
		parsed.cmdmsg = parsed.msg.substr(0, parsed.msg.find(" "));		//get the message but cut it off at the first space
		return parsed;
	}

	// --- /ping returns 'PONG' to the client it was sent from ---
	bool Ping(SOCKET sock, ParsedMSG parsed) {
		if (parsed.msg == "/ping") {
			std::cout << "Command Called: /ping" << std::endl;
			char ping[] = "PONG";
			send(sock, ping, 5, 0);
			return true;
		}
		return false;
	}

	// --- /quit tells others in the room that the client is disconnecting and sends a msg to client to confirm ---
	bool Quit(SOCKET sock, ParsedMSG parsed, fd_set& set, std::string roomName, SOCKET* listening) {
		if (parsed.msg == "/quit") {
			std::cout << "Command Called: /quit" << std::endl;
			char ping[] = "PONG";
			send(sock, ping, 5, 0);
			std::string broadcast = "SERVER: " + parsed.username + " is leaving";
			std::cout << broadcast << std::endl;

			if (roomName != "Master")
				SendMSG(sock, *listening, broadcast, set);

			return true;
		}
		return false;
	}

	// --- /silent does nothing... litarally nothing, it can be usefull, maybe ---
	bool Silent(ParsedMSG parsed) {
		if (parsed.cmdmsg == "/silent") {
			std::cout << "Command Called: /silent" << std::endl;
			return true;
		}
		return false;
	}

	// --- /echo is similar to /ping exept instead of returning 'Pong' it returns whatever the client sends after it ---
	// example: client sends '/echo hello' server responds 'hello'
	bool Echo(SOCKET sock, ParsedMSG parsed) {
		if (parsed.cmdmsg == "/echo") {
			std::cout << "Command Called: /echo" << std::endl;
			if (parsed.msg.length() > 6) {
				std::string str = parsed.msg.substr(6, parsed.msg.length());
				std::cout << str << std::endl;
				send(sock, str.c_str(), 5, 0);
			}
			return true;
		}
		return false;
	}

	// --- /changename notifys others in the room that the client is changing their username and what their changing it to ---
	bool Changename(SOCKET sock, ParsedMSG parsed, fd_set& set, std::string roomName, SOCKET* listening) {
		if (parsed.cmdmsg == "/changename") {
			std::cout << "Command Called: /changename" << std::endl;
			std::string str = parsed.msg.substr(12, parsed.msg.length());
			std::string toSend = "SERVER: " + parsed.username + " has changed there name to " + str;
			if (roomName != "Master")
				SendMSG(sock, *listening, toSend, set);

			return true;

		}
		return false;
	}

	// --- /list lists all rooms ---
	bool List(SOCKET sock, ParsedMSG parsed, RoomList* Rooms) {
		if (parsed.msg == "/list") {
			std::cout << "Command Called: /list" << std::endl;
			std::string message = "SERVER: Chat rooms are:";
			for (int i = 0; i < Rooms->GetSize(); ++i) {
				message = message + "\n" + Rooms->Read(i).roomName + "\t" + std::to_string(Rooms->Read(i).roomSet.fd_count);
			}
			send(sock, message.c_str(), message.size() + 1, 0);
			return true;
		}
		return false;
	}

	// --- /createroom creates a room in a new thread if the entered name is availible, than auto-joins it, and notifys every on in the room that you are in that you are leaving ---
	bool Createroom(SOCKET sock, ParsedMSG parsed, fd_set& set, RoomList* Rooms, std::string roomName, SOCKET* listening) {
		if (parsed.cmdmsg == "/createroom") {
			std::cout << "Command Called: /createroom" << std::endl;
			if (parsed.msg.length() > 15 && parsed.msg.length() < 22) {
				std::string str = parsed.msg.substr(12, parsed.msg.length());
				for (int i = 0; i < Rooms->GetSize(); ++i) {
					if (str == Rooms->Read(i).roomName || str == "Main") {
						std::string message = "SERVER: Hey, that name is taken!!!!\nPick a different one please!";
						send(sock, message.c_str(), message.size() + 1, 0);
						return true;
					}
				}
				if (roomName != "Master") {
					std::string toSend = "SERVER: " + parsed.username + " is leaving the room to go to another\nroom that is better than this one";
					SendMSG(sock, *listening, toSend, set);
				}
				//create the room
				Rooms->Write(&(Room(str)));
				Rooms->Read(0).Add(sock, set);
				//start the thread for the room
				std::thread T(&Room::Process, std::ref(Rooms->Read(0)), std::ref(*Rooms));
				T.detach();
				return true;
			}
			else if (parsed.msg.length() <= 12) {
				//no room name entered
				std::string message = "SERVER: Hey!, please enter the room name in your command\nexample: /createroom myroom";
				send(sock, message.c_str(), message.size() + 1, 0);
			}
			else if (parsed.msg.length() > 15) {
				//room name too big
				std::string message = "SERVER: Hey!, that name is too long...";
				send(sock, message.c_str(), message.size() + 1, 0);
			}
			else {
				//room name too small
				std::string message = "SERVER: Hey!, that name is too short...";
				send(sock, message.c_str(), message.size() + 1, 0);
			}
		}
		return false;
	}

	// --- /join joins the selected room and notifys others in the room that your joining, and notifys others in the room that your leaving ---
	bool Join(SOCKET sock, ParsedMSG parsed, fd_set& set, RoomList* Rooms, std::string roomName, SOCKET* listening) {
		if (parsed.cmdmsg == "/join") {
			std::cout << "Command Called: /join" << std::endl;
			std::string str = parsed.msg.substr(6, parsed.msg.length());
			if (str == roomName) {
				std::string message = "SERVER: Hey! You're already in that room!";
				send(sock, message.c_str(), message.size() + 1, 0);
				return true;
			}

			for (int i = 0; i < Rooms->GetSize(); ++i) {
				if (str == Rooms->Read(i).roomName) {
					//add client to the room
					std::string message = "SERVER: Room found... joining";
					send(sock, message.c_str(), message.size() + 1, 0);
					Rooms->Read(i).Add(sock, set);
					if (roomName != "Master") {
						std::string toSend = "SERVER: " + parsed.username + " is leaving the room to go to another\nroom that is better than this one";
						SendMSG(sock, *listening, toSend, set);
					}
					std::string toSend = "SERVER: " + parsed.username + " is joining the room";
					SendMSG(sock, *listening, toSend, Rooms->Read(i).roomSet);
					return true;
				}
			}
			//if you make it here you tried to join a room that does not exist
			std::string message = "SERVER: Hey! that room doesn't exist!!!!!";
			send(sock, message.c_str(), message.size() + 1, 0);
		}
		return false;
	}

	// --- /help sends the contents of a help text file to the client ---
	bool Help(SOCKET sock, ParsedMSG parsed) {
		if (parsed.msg == "/help") {
			std::cout << "Command Called: /help" << std::endl;
			//read help file and send contents to client
			std::ifstream ifst;
			ifst.open("help.txt");
			std::string line;
			if (ifst.is_open()) {
				while (!ifst.eof()) {
					//read and send the file line by line
					std::cout << "READING LINE" << std::endl;
					//read the line
					std::getline(ifst, line);
					//convert the line from "'LINE'" to "SERVER: 'LINE'\n"
					line = "SERVER: " + line + "\n";
					std::cout << line;
					std::cout << "FROM help.txt" << std::endl;
					//send the line
					send(sock, line.c_str(), line.length() + 1, 0);
				}
			}
			return true;

		}
		return false;
	}

}