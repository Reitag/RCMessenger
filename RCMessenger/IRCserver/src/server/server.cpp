#include <WinSock2.h>
#include "server.h"

// since these variables needs to be defined somewhere,
// I define them in this .cpp file
WSAData wsData;
fd_set master;
SOCKET serverSocket;
SOCKET clientSocket;
in_addr IPinAddr;
sockaddr_in serverSockaddr;
sockaddr_in clientSockaddr;

void establsihNetwork(const int& family, const int& type, const std::string& IPv4, const int& port, const int& maxQueue, const int& flag)
{
	short checkStatus = 0;

	//winSock
	checkStatus = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (checkStatus != 0)
	{
		std::cerr << "Error WinSock version initializaion #" << WSAGetLastError() << std::endl;
		return;
	}
	//array for sockets
	FD_ZERO(&master);

	//server socket
	serverSocket = socket(family, type, flag);
	if (serverSocket == INVALID_SOCKET)
	{
		std::cerr << "Error initialization socket # " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	//IP
	checkStatus = inet_pton(family, IPv4.c_str(), &IPinAddr);
	if (checkStatus <= 0)
	{
		std::cerr << "Error in IP translation to special numeric format" << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}

	//establish server info
	serverSockaddr.sin_family = family;
	serverSockaddr.sin_addr = IPinAddr;
	serverSockaddr.sin_port = htons(port);

	//bind server socket
	checkStatus = bind(serverSocket, (sockaddr*)&serverSockaddr, sizeof(serverSockaddr));
	if (checkStatus != 0)
	{
		std::cerr << "Error Socket binding to server info. Error # " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}
	//link array with server socket
	FD_SET(serverSocket, &master);

	//listening
	checkStatus = listen(serverSocket, maxQueue);
	if (checkStatus != 0)
	{
		std::cerr << "Can't start to listen to. Error # " << WSAGetLastError() << std::endl;
		closesocket(serverSocket);
		WSACleanup();
		return;
	}
}


//create client socket
bool connectClient()
{
	//clear vars in case of previouse client
	ZeroMemory(&clientSockaddr, sizeof(clientSockaddr));

	int clientSize = sizeof(clientSockaddr);
	clientSocket = accept(serverSocket, (sockaddr*)&clientSockaddr, &clientSize);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "Client detected, but can't connect to a client. Error # " << WSAGetLastError() << std::endl;
		closesocket(clientSocket);

		return false; 
	}
	else
	{
		return true;
	}
}

//drop client
void dropClient(SOCKET& clientSock)
{
	closesocket(clientSock);
}

//receive log and pass
bool receiveLogAndPass(std::string& log, std::string& pass, SOCKET& client)
{
	int bytesIn = 0;
	char dataIn[20];
	std::string _msg;

	_msg = "Input your login\r\n";
	send(client, _msg.c_str(), static_cast<int>(_msg.length()), 0);
	//receive log
	ZeroMemory(dataIn, sizeof(dataIn));
	bytesIn = recv(client, dataIn, sizeof(dataIn), 0);
	if (!(checkBytesIn(bytesIn)))
	{
		return false;
	}
	dataFromClient(dataIn, bytesIn, log);

	_msg = "Input your password\r\n";
	send(client, _msg.c_str(), static_cast<int>(_msg.length()), 0);
	//receive pass
	ZeroMemory(dataIn, sizeof(dataIn));
	bytesIn = recv(client, dataIn, sizeof(dataIn), 0);
	if (!(checkBytesIn(bytesIn)))
	{
		return false;
	}
	dataFromClient(dataIn, bytesIn, pass);
	
	//clear dataIn
	ZeroMemory(dataIn, sizeof(dataIn));

	return true;
}

void dataFromClient(char* data, const int& bytes, std::string& value)
{
	for (int i = 0; i < bytes; ++i)
	{
		if (data[i] == '\0' || data[i] == '\n' || data[i] == '\r')
		{
			break;
		}
		else
		{
			value += data[i];
		}
	}
}

bool checkBytesIn(const int& bytes)
{
	if (bytes == SOCKET_ERROR)
	{
		std::cerr << "Error in recv: " << WSAGetLastError() << std::endl;

		return false;
	}
	else if (bytes == 0)
	{
		std::cerr << "Connection closed by client." << std::endl;

		return false;
	}

	return true;
}

void sendLastMsg(SOCKET& client)
{
	std::string lastMsg;

	lastMsg = "See you soon!\r\n";
	send(client, lastMsg.c_str(), static_cast<int>(lastMsg.length()), 0);
}