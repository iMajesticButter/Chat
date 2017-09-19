#include "main.h"

//process all incoming messages (relay to other clients, parse commands, etc...)
void ProcessIncomingMSG(SOCKET sock, fd_set& set, SOCKET listening, std::string RoomName, RoomList& Rooms) {

	//create a buffer to hold receved message
	char buf[4096];
	ZeroMemory(buf, 4096);

	//receve the message and put it into a char array
	int bytesIn = recv(sock, buf, 4096, 0);

	//a simple struct to get data out of the 'ParseAndCommands' function
	ParsedMSG Parsed;

	if (bytesIn > 0) {
		//parse the massage and run any commands entered sutch as /join, of /createroom etc...
		Parsed = ParseAndCommands(sock, listening, set, buf, Rooms, RoomName);
	}

	//if the message was empty
	if (bytesIn <= 0) {
		//drop the client
		closesocket(sock);
		FD_CLR(sock, &set);
		return;
	}
	//if the message was not empty and no commands where found in 'ParseAndCommands'
	else if(!Parsed.command) {
		//send message to other clients
		std::string sbuf = buf;
		SendMSG(sock, listening, sbuf, set);
	}
	//write message to console
	std::cout << buf << std::endl;
}

//process all clients that are not connected to a room
void ProcessMaster(SOCKET sock, fd_set& set, SOCKET listening, RoomList& Rooms) {

	//create a buffer to hold receved message
	char buf[4096];
	ZeroMemory(buf, 4096);

	//receve the message and put it into a char array
	int bytesIn = recv(sock, buf, 4096, 0);

	//if the message actually contains somthing
	if (bytesIn > 0) {
		
		//a simple struct to get data out of the 'ParseAndCommands' function
		ParsedMSG Parsed;

		//parse the massage and run any commands entered sutch as /join, of /createroom etc...
		Parsed = ParseAndCommands(sock, listening, set, buf, Rooms, "Master");
		
	}
	//if the message was empty
	else {
		//drop client
		closesocket(sock);
		FD_CLR(sock, &set);
		return;
	}
	std::cout << buf << std::endl;
}

//send message to all but one client and not to the listening socket
void SendMSG(SOCKET sock, SOCKET listening, std::string str, fd_set set) {

	//loop thru all sockets in the input set
	for (int i = 0; i < set.fd_count; ++i) {
		//'outSock' is the socket on the current iteration of the loop
		SOCKET outSock = set.fd_array[i];
		//if 'outSock' is not the listening socket or the socket that is being excluded
		if (outSock != listening && outSock != sock) {
			//send the message to that socket
			send(outSock, str.c_str(), str.length() + 1, 0);
		}
	}

}

//parse a massage and run any commands entered sutch as /join, of /createroom etc...
ParsedMSG ParseAndCommands(SOCKET sock, SOCKET listening, fd_set& set, char buf[], RoomList& Rooms, std::string roomName) {

	//parse
	std::string s = buf;												//convert buf to string
	std::string del = "> ";												//the "> " after every username
	std::string username = s.substr(0, s.find(del));					//remove message and "> " leaving just the username
	std::string msg = s.substr(s.find(del) + del.length(), s.length());	//remove username and "> " leaving just the message
	std::string cmdmsg = msg.substr(0, msg.find(" "));					//get the message but cut it off at the first space

	ParsedMSG toReturn;
	toReturn.username = username;
	toReturn.msg = msg;
	toReturn.command = false;
	//check for any commands
	// --- /ping returns 'PONG' to the client it was sent from ---
	if (msg == "/ping") {
		std::cout << "Command Called: /ping" << std::endl;
		char ping[] = "PONG";
		send(sock, ping, 5, 0);
		toReturn.command = true;
		return toReturn;
	}
	// --- /quit tells others in the room that the client is disconnecting and sends a msg to client to confirm ---
	else if (msg == "/quit") {
		std::cout << "Command Called: /quit" << std::endl;
		char ping[] = "PONG";
		send(sock, ping, 5, 0);
		std::string broadcast = "SERVER: " + username + " is leaving";
		std::cout << broadcast << std::endl;

		if (roomName != "Master")
			SendMSG(sock, listening, broadcast, set);

		toReturn.command = true;

		return toReturn;
	}
	// --- /silent does nothing... litarally nothing, it can be usefull, maybe ---
	else if (cmdmsg == "/silent") {
		std::cout << "Command Called: /silent" << std::endl;
		toReturn.command = true;
		return toReturn;
	}
	// --- /echo is similar to /ping exept instead of returning 'Pong' it returns whatever the client sends after it ---
	// example: client sends '/echo hello' server responds 'hello'
	else if (cmdmsg == "/echo") {
		std::cout << "Command Called: /echo" << std::endl;
		if (msg.length() > 6) {
			std::string str = msg.substr(6, msg.length());
			std::cout << str << std::endl;
			send(sock, str.c_str(), 5, 0);
		}

		toReturn.command = true;

		return toReturn;
	}
	// --- /changename notifys others in the room that the client is changing their username and what their changing it to ---
	else if (cmdmsg == "/changename") {
		std::cout << "Command Called: /changename" << std::endl;
		std::string str = msg.substr(12, msg.length());
		std::string toSend = "SERVER: " + username + " has changed there name to " + str;
		if(roomName != "Master")
			SendMSG(sock, listening, toSend, set);

		toReturn.command = true;
		return toReturn;

	}
	// /list lists all rooms
	else if (msg == "/list") {
		std::string message = "SERVER: Chat rooms are:";
		for (int i = 0; i < Rooms.GetSize(); ++i) {
			message = message + "\n" + Rooms.Read(i).roomName + "\t" + std::to_string(Rooms.Read(i).roomSet.fd_count);
		}
		send(sock, message.c_str(), message.size() + 1, 0);
		toReturn.command = true;
		return toReturn;
	}
	// --- /createroom creates a room in a new thread if the entered name is availible, than auto-joins it, and notifys every on in the room that you are in that you are leaving ---
	else if (cmdmsg == "/createroom") {
		if (msg.length() > 12) {
			std::string str = msg.substr(12, msg.length());
			for (int i = 0; i < Rooms.GetSize(); ++i) {
				if (str == Rooms.Read(i).roomName || str == "Main") {
					std::string message = "SERVER: Hey, that name is taken!!!!\nPick a different one please!";
					send(sock, message.c_str(), message.size() + 1, 0);
					toReturn.command = true;
					return toReturn;
				}
			}
			if (roomName != "Master") {
				std::string toSend = "SERVER: " + username + " is leaving the room to go to another\nroom that is better than this one";
				SendMSG(sock, listening, toSend, set);
			}
			//create the room
			Rooms.Write(&(Room(str)));
			Rooms.Read(0).Add(sock, set);
			//start the thread for the room
			std::thread T(&Room::Process, std::ref(Rooms.Read(0)), std::ref(Rooms));
			T.detach();
			toReturn.command = true;
			return toReturn;
		}
		else {
			std::string message = "SERVER: Hey!, please enter the room name in your command\nexample: /createroom myroom";
			send(sock, message.c_str(), message.size() + 1, 0);
		}
	}
	// --- /join joins the selected room and notifys others in the room that your joining, and notifys others in the room that your leaving ---
	else if (cmdmsg == "/join") {
		std::string str = msg.substr(6, msg.length());
		if (str == roomName) {
			std::string message = "SERVER: Hey! You're already in that room!";
			send(sock, message.c_str(), message.size() + 1, 0);
			toReturn.command = true;
			return toReturn;
		}
		
		for (int i = 0; i < Rooms.GetSize(); ++i) {
			if (str == Rooms.Read(i).roomName) {
				//add client to the room
				std::string message = "SERVER: Room found... joining";
				send(sock, message.c_str(), message.size() + 1, 0);
				Rooms.Read(i).Add(sock, set);
				if (roomName != "Master") {
					std::string toSend = "SERVER: " + username + " is leaving the room to go to another\nroom that is better than this one";
					SendMSG(sock, listening, toSend, set);
				}
				std::string toSend = "SERVER: " + username + " is joining the room";
				SendMSG(sock, listening, toSend, Rooms.Read(i).roomSet);
				toReturn.command = true;
				return toReturn;
			}
		}
		//if you make it here you tried to join a room that does not exist
		std::string message = "SERVER: Hey! that room doesn't exist!!!!!";
		send(sock, message.c_str(), message.size() + 1, 0);
	}
	toReturn.command = false;
	return toReturn;

}

//process everyone in the master set
void Master(fd_set& set, SOCKET listening, RoomList& Rooms) {
	//get input on whether or not to use rooms
	bool useRooms = false;
	std::string ST = "";
	while (true) {
		std::cout << "should the server use rooms (better than not useing rooms)" << std::endl;
		std::cin >> ST;
		if (ST == "yes" || ST == "y") {
			//use rooms
			std::cout << "rooms activated" << std::endl;
			useRooms = true;
			break;
		}
		else if (ST == "no" || ST == "n") {
			//don't use rooms
			std::cout << "running without rooms" << std::endl;
			break;
		}
		else {
			std::cout << "INVALID RESPONSE TRY AGAIN" << std::endl;
		}
	}
	//main server loop
	while (true) {
		//copy the set so it won't get messed with by select()
		fd_set copy = set;

		std::cout << "clients in master set: " << set.fd_count << std::endl;

		//run select to wait until message receved
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		std::cout << "message receved on master set!" << std::endl;

		std::cout << "number of rooms: " << Rooms.GetSize() << std::endl;

		//process everyone in the master set who has sent a message
		for (int i = 0; i < socketCount; ++i) {
			//sock is the socket currently being processed in this iteration of the for loop
			SOCKET sock = copy.fd_array[i];
			//if message was receved on the listening socket
			if (sock == listening) {
				//accept new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				//add the new connection to list of connected clients
				FD_SET(client, &set);

				//send welcome message to new client
				std::string welcomeMsg = "SERVER: Welcome to the server! please select a chat room\nChat rooms are:";
				
				for (i = 0; i < (Rooms).GetSize(); ++i) {
					welcomeMsg = welcomeMsg + "\n" + Rooms.Read(i).roomName + "\t" + std::to_string(Rooms.Read(i).roomSet.fd_count);
				}
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
			}
			else {
				//if message was not sent on the listening socket, process normally
				if(!useRooms)
					//just mash everyone into one big single threaded room
					ProcessIncomingMSG(sock, set, listening, "Main", Rooms);
				else
					//run commands to join, create, and list rooms
					ProcessMaster(sock, set, listening, Rooms);
			}
		}
	}
}