#include "main.h"

int main() {

	int portNumber = 54000;

	//initialize winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2,2);
	
	int wsOk = WSAStartup(ver, &wsData);
	//if we failed to initialize winSock
	if (wsOk != 0) {
		//ERROR
		std::cerr << "Can't Init Winsock" << std::endl;
		return -1;
	}

	//create the listening socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	//if we failed to create the listening socket
	if (listening == INVALID_SOCKET) {
		//ERROR
		std::cerr << "Can't create listening socket" << std::endl;
		return -1;
	}

	//bind ip and port to the listening socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(portNumber);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(listening, (sockaddr*)&hint, sizeof(hint));

	//tell winsock the listening socket is for listening
	listen(listening, SOMAXCONN);

	//create the master set of sockets
	fd_set master;
	FD_ZERO(&master);

	//add the listening socket to the master set
	FD_SET(listening, &master);


	//create the list of rooms
	RoomList Rooms;

	//create a list of usernames
	Namelist Usernames;

	//run the server loop and all the processing that comes with it
	Master(master, listening, Rooms, &Usernames);

	//cleanup winsock
	WSACleanup();

	//return SUCSESS!!!!
	return 1;
}












//wait 4 connection
//sockaddr_in client;
//int clientsize = sizeof(client);
//SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientsize);
//if (clientSocket == INVALID_SOCKET) {
//	std::cerr << "HEY! that client is invalid, I quit!" << std::endl;
//	return;
//}

//char host[NI_MAXHOST];		//client's remote name
//char service[NI_MAXSERV];	//service the client can connect on

//ZeroMemory(host, NI_MAXHOST);
//ZeroMemory(service, NI_MAXSERV);

//if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
//	std::cout << host << " connected on port " << service << std::endl;
//}
//else {
//	inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
//	std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
//}

////close listening socket
//closesocket(listening);

////do chat stuff
//char buf[4096];

//while (true) {
//	
//	ZeroMemory(buf, 4096);

//	//wait 4 client data
//	int bytesreceved = recv(clientSocket, buf, 4096, 0);
//	if (bytesreceved == SOCKET_ERROR) {
//		std::cerr << "error in recv()" << std::endl;
//		break;
//	}

//	if (bytesreceved == 0) {
//		std::cout << "clitent disconnected" << std::endl;
//		break;
//	}

//	//echo the message back
//	send(clientSocket, buf, bytesreceved + 1, 0);
//	std::cout << "client> " << std::string(buf, 0, bytesreceved) << std::endl;
//}