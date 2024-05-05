#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "Ws2_32.lib")
#include "server/server.h"

#include <sstream>
#include <unordered_map>
#include <thread>
#include <mutex>

#define BFSIZE 1024

//Database parameters
constexpr const char* host = "127.0.0.1";
constexpr const char* admin = "root";
constexpr const char* pass = "pass";
constexpr const char* name = "chat";
constexpr const char* table = "accounts";

//Connection parameters
constexpr const int family = AF_INET;
constexpr const int type = SOCK_STREAM;
constexpr const char* IP = "0.0.0.0";
constexpr const int port = 55555;
constexpr const int maxQueue = SOMAXCONN;
constexpr const int flag = 0;

//uo_maps
std::unordered_map<SOCKET, std::string> users;
std::unordered_map<SOCKET, bool> socketFlag;

//thread
std::mutex mutex;

void clientHandling(SOCKET clientSock, MySQL_API& sql)
{
	//variables
	char buffer[BFSIZE]{};
	int bytesIn = 0;
	std::string login, password, queryToDB;
	bool open = true;

	//greeting message
	std::string msg = "SERVER: This is IRCserver. Input one of futher commands\r\n1 - enter account;\r\n2 - create new account;\r\n0 - exit.\r\nChose option:\r\n";
	send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);

	while (open)
	{
		ZeroMemory(buffer, sizeof(buffer));
		bytesIn = recv(clientSock, buffer, sizeof(buffer), 0);
		if (bytesIn == SOCKET_ERROR)
		{
			return;
		}
		char choice = buffer[0];
		switch (choice)
		{
		case '1':
		{
			msg = "Option [1]: ";
			send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);

			if (!(receiveLogAndPass(login, password, clientSock)))
			{
				return;
			}

			// Check for user online
			bool userOnline = false;
			for (std::unordered_map<SOCKET, std::string>::const_iterator it = users.begin(); it != users.end(); ++it)
			{
				if (it->second == login)
				{
					userOnline = true;
					break;
				}
			}
			// If user online - exit from case
			if (userOnline)
			{
				login.clear();
				password.clear();
				msg = "SERVER: Error! This user is online!\r\nChose option:\r\n";
				send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);
				break;
			}

			queryToDB = "SELECT login, password FROM " + std::string(table) + " WHERE login = \'" + login + "\' and password = \'" + password + "\'";
			
			//mutex to avoid thread's races for global variables
			mutex.lock();

			if (sql.selectFromDB(queryToDB) == true)
			{
				msg = "SERVER: Welcome to chat, young padavan! For exit from chat type \'ext\'\r\n--------------------\r\n";
				send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);

				//set socket with user's login and his flag on true to receive messages
				socketFlag[clientSock] = true;
				users[clientSock] = login;
				
				//break the loop
				open = false;
			}
			else
			{
				msg = "SERVER: Access denied. Login or password are incorrect.\r\nChose option:\r\n";
				send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);
			}

			//unlock mutex
			mutex.unlock();

			login.clear();
			password.clear();
			queryToDB.clear();
			

			break;
		}
		case '2':
		{
			msg = "Option [2]: ";
			send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);

			if (!(receiveLogAndPass(login, password, clientSock)))
			{
				return;
			}
			queryToDB = "SELECT login FROM " + std::string(table) + " WHERE login = \'" + login + "\'";

			//mutex lock to avoid thread's races
			mutex.lock();
			if (sql.selectFromDB(queryToDB) == true)
			{
				msg = "SERVER: Error! This login exist.\r\nChose option:\r\n";
				send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);
			}
			else
			{
				queryToDB.clear();
				queryToDB = "INSERT INTO " + std::string(table) + " (login, password) VALUES(\'" + login + "\', \'" + password + "\')";

				if (sql.promptToDB(queryToDB) == true)
				{
					msg = "SERVER: Account has been created. Welcome to chat young padavan. For exit from chat type \'ext\'\r\n";
					send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);

					//set socket with user's login and his flag on true to receive messages
					socketFlag[clientSock] = true;
					users[clientSock] = login;

					//break the loop
					open = false;
				}
				else
				{
					msg = "SERVER: Error! Account has not been created.\r\nChose option:\r\n";
					send(clientSock, msg.c_str(), static_cast<int>(msg.length()), 0);
				}
			}

			//mutex unlock
			mutex.unlock();

			login.clear();
			password.clear();

			break;
		}
		case '0':
		{
			sendLastMsg(clientSock);
			dropClient(clientSock);

			//break the loop
			open = false;

			break;
		}
		default:
		{
			break;
		}
		}
	}
}

int main()
{
	//create sql object with connection to db
	MySQL_API sql(name, host, admin, pass);

	//start server listening on 55555 port
	establsihNetwork(family, type, IP, port, maxQueue, flag);

	//variables
	char buffer[BFSIZE]{};
	int socketCount;
	std::string message;
	std::ostringstream ss;
	std::string strOut;

	while (true)
	{
		fd_set copy = master;
		socketCount = select(0, &copy, nullptr, nullptr, nullptr);
		for (int i = 0; i < socketCount; ++i)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == serverSocket)
			{
				if (connectClient() == false)
				{
					//Drop client
					dropClient(sock);
				}
				else
				{
					//adding socket to master's array and in uo_map with flags
					FD_SET(clientSocket, &master);
					socketFlag[clientSocket] = false;

					//since clientScoket is one variable for all threads, it is important to move this object to handler
					//to avoid thread's races
					std::thread clientThread(clientHandling, std::move(clientSocket), std::ref(sql));
					clientThread.detach();
				}
			}
			else
			{
				ZeroMemory(buffer, sizeof(buffer));
				//recieve msg
				int bytesIn = recv(sock, buffer, sizeof(buffer), 0);
				if (bytesIn <= 0)
				{
					//remove client's socket from master's array
					FD_CLR(sock, &master);

					//remove socket from uo_maps
					users.erase(sock);
					socketFlag.erase(sock);

					//Drop client
					dropClient(sock);
				}
				else
				{
					for (size_t k = 0; k < BFSIZE && buffer[k] != '\0'; ++k)
					{
						message += buffer[k];
					}
					ss << "[" << users[sock] << "]: " << message;
					strOut = ss.str();

					//drop client if exit
					if (message[0] == 'e' && message[1] == 'x' && message[2] == 't')
					{
						sendLastMsg(sock);
						strOut.clear();
						strOut = "SERVER: " + users[sock] + " has left the building!\r\n";

						//remove client's socket from master's array
						FD_CLR(sock, &master);

						//remove socket from uo_maps
						users.erase(sock);
						socketFlag.erase(sock);

						//drop client
						dropClient(sock);
					}
					//send msg to other clients
					for (size_t j = 0; j < master.fd_count; ++j)
					{
						SOCKET outSock = master.fd_array[j];
						if (outSock != serverSocket /* && outSock != sock*/ && socketFlag[outSock] == true)
						{
							send(outSock, strOut.c_str(), static_cast<int>(strOut.size()) + 1, 0);
						}
					}
					strOut.clear();
					message.clear();
					ss.str("");
				}
			}
		}
	}

	closesocket(serverSocket);
	WSACleanup();

	return 0;
}