#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

void InputSend(SOCKET, bool*);

int main() {


	std::string ipAddress = "127.0.0.1";
	int port = 54000;

	//get ip to connect to
	std::cout << "what IP would you like to connect to?" << std::endl;
	std::cin >> ipAddress;

	//init winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		std::cerr << "Can't Init Winsock" << std::endl;
		return -1;
	}

	//create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Can't create socket! Err #" << WSAGetLastError() << std::endl;
		return -1;
	}
	
	//fill in hint struct
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	//connect to server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR) {
		std::cerr << "can't connect to server, Err #" << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	//send and receve data
	char buf[4096];
	bool stop = false;

	//Prompt the user for some text in a sepreat thread
	std::thread prompt(InputSend, sock, &stop);

	do {
		//wait 4 responce
		int bytesReceved = recv(sock, buf, 4096, 0);
		if (bytesReceved > 0) {
			//echo responce to console
			std::cout << std::string(buf, 0, bytesReceved) << std::endl;
		}

	} while (stop == false);
	//close down everything
	prompt.join();
	closesocket(sock);
	WSACleanup();

	return 1;
}

void InputSend(SOCKET sock, bool* stop) {

	while (*stop == false){
		std::string userInput;
		std::getline(std::cin, userInput);

		std::string cmdmsg = userInput.substr(0, userInput.find(" "));	//get the user input but cut it off at the first space

		std::string toSend = userInput;

		if (userInput == "/quit") {
			*stop = true;
		}
		if (userInput.size() > 0) {
			//send da txt
			int sendResult = send(sock, toSend.c_str(), toSend.size() + 1, 0);
		}
	}
}