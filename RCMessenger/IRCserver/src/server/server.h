#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "../mysql-api/mysql-api.h"

// network

// extern is for remove link error
// they are declared in .cpp file
extern WSAData wsData;
extern fd_set master;
extern SOCKET serverSocket;
extern SOCKET clientSocket;
extern in_addr IPinAddr;
extern sockaddr_in serverSockaddr;
extern sockaddr_in clientSockaddr;

void establsihNetwork(const int& family, const int& type, const std::string& IPv4, const int& port, const int& maxQueue, const int& flag);
bool connectClient();
void dropClient(SOCKET& clientSock);
bool receiveLogAndPass(std::string& log, std::string& pass, SOCKET& client);
void dataFromClient(char* data, const int& bytes, std::string& value);
bool checkBytesIn(const int& bytes);
void sendLastMsg(SOCKET& client);