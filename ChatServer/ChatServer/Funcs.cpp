#include "main.h"

//process all incoming messages (relay to other clients, parse commands, etc...)
void ProcessIncomingMSG(SOCKET sock, fd_set& set, SOCKET listening, std::string RoomName, RoomList& Rooms) {

	//create a buffer to hold receved message
	char buf[4096];
	ZeroMemory(buf, 4096);

	//receve the message and put it into a char array
	int bytesIn = recv(sock, buf, 4096, 0);

	//a simple struct to get data out of the 'ParseAndCommands' function
	chatcmd::ParsedMSG Parsed;

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
		chatcmd::ParsedMSG Parsed;

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
chatcmd::ParsedMSG ParseAndCommands(SOCKET sock, SOCKET listening, fd_set& set, char buf[], RoomList& Rooms, std::string roomName) {

	chatcmd::ParsedMSG toReturn = chatcmd::Parse(buf);
	

	toReturn.command = false;
	//check for any commands
	// --- /ping returns 'PONG' to the client it was sent from ---
	if (chatcmd::Ping(sock, toReturn)) {
		toReturn.command = true;
		return toReturn;
	}
	// --- /quit tells others in the room that the client is disconnecting and sends a msg to client to confirm ---
	else if (chatcmd::Quit(sock, toReturn, set, roomName, &listening)) {
		toReturn.command = true;
		return toReturn;
	}
	// --- /silent does nothing... litarally nothing, it can be usefull, maybe ---
	else if (chatcmd::Silent(toReturn)) {
		toReturn.command = true;
		return toReturn;
	}
	// --- /echo is similar to /ping exept instead of returning 'Pong' it returns whatever the client sends after it ---
	// example: client sends '/echo hello' server responds 'hello'
	else if (chatcmd::Echo(sock, toReturn)) {
		toReturn.command = true;
		return toReturn;
	}
	// --- /changename notifys others in the room that the client is changing their username and what their changing it to ---
	else if (chatcmd::Changename(sock, toReturn, set, roomName, &listening)) {
		toReturn.command = true;
		return toReturn;
	}
	// --- /list lists all rooms ---
	else if (chatcmd::List(sock, toReturn, &Rooms)) {
		toReturn.command = true;
		return toReturn;
	}
	// --- /createroom creates a room in a new thread if the entered name is availible, than auto-joins it, and notifys every on in the room that you are in that you are leaving ---
	else if (chatcmd::Createroom(sock, toReturn, set, &Rooms, roomName, &listening)) {
		toReturn.command = true;
		return toReturn;
	}
	// --- /join joins the selected room and notifys others in the room that your joining, and notifys others in the room that your leaving ---
	else if (chatcmd::Join(sock, toReturn, set, &Rooms, roomName, &listening)) {
		toReturn.command = true;
		return toReturn;
	}
	// --- /help sends the contents of a help text file to the client ---
	else if (chatcmd::Help(sock, toReturn)) {
		toReturn.command = true;
		return toReturn;
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
				//welcome message is:
				/*
				SERVER: Welcome to the server! please select a chat room
				Chat rooms are:
example			room		1
example			blaRoom		1
example			ajhahsdfhas	50
example			myroom		8
example			datroom		77
example			jakes r		8
example			bobroom		9
example			dabest		9
				

				use /createroom 'room name' to create a new room
				use /join 'room name' to join a room
				use /list to get a list of rooms
				use /help if you need help... that one is kinda self explanatory
				*/
				std::string welcomeMsg = "SERVER: Welcome to the server! please select a chat room\nChat rooms are:";
				
				for (int l = 0; l < (Rooms).GetSize(); ++l) {
					if(Rooms.Read(l).roomName.length() < 7)
						welcomeMsg = welcomeMsg + "\n" + Rooms.Read(l).roomName + "\t\t" + std::to_string(Rooms.Read(l).roomSet.fd_count);
					else
						welcomeMsg = welcomeMsg + "\n" + Rooms.Read(l).roomName + "\t" + std::to_string(Rooms.Read(l).roomSet.fd_count);
				}
				welcomeMsg += "\n\nuse /createroom 'room name' to create a new room\nuse /join 'room name' to join a room\nuse /list to get a list of rooms\nuse /help if you need help... that one is kinda self explanatory";
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