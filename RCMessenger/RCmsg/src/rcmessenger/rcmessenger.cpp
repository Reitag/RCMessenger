#include <WinSock2.h>
#include "rcmessenger.h"

WSAData wsData;
SOCKET clientSocket;
in_addr IPinAddr;
sockaddr_in serverSockaddr;
sockaddr_in clientSockaddr;
HINSTANCE hInst;
HANDLE threadMsg;
HWND ipHwnd;
HWND hTextArea;
HWND hMsg;
HFONT font;
bool wndControl = false;

bool createConnect(HWND& hWnd, const int& family, const int& type, const std::string& IPv4, const int& port, const int& flag)
{
	short checkStatus = 0;
	std::string error;
	// init WinSock
	checkStatus = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (checkStatus != 0)
	{
		error = "Error WinSock version initializaion" + std::string("\r\n");
		setText(hWnd, error);
		return false;
	}

	// create client socket
	clientSocket = socket(family, type, flag);
	if (clientSocket == INVALID_SOCKET)
	{
		error = "Error initialization socket" + std::string("\r\n");
		setText(hWnd, error);
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}

	// inti IP
	checkStatus = inet_pton(family, IPv4.c_str(), &IPinAddr);
	if (checkStatus <= 0)
	{
		error = "Error in IP translation to special numeric format" + std::string("\r\n");
		setText(hWnd, error);
		return false;
	}

	// establish server info
	ZeroMemory(&serverSockaddr, sizeof(serverSockaddr));
	serverSockaddr.sin_family = family;
	serverSockaddr.sin_addr = IPinAddr;
	serverSockaddr.sin_port = htons(port);

	// connect to server
	checkStatus = connect(clientSocket, (sockaddr*)&serverSockaddr, sizeof(serverSockaddr));
	if (checkStatus != 0)
	{
		error = "Connection to Server is FAILED." + std::string("\r\n");
		setText(hWnd, error);
		closesocket(clientSocket);
		WSACleanup();
		return false;
	}

	return true;
}

void sendMessages()
{
	int send_data;
	std::string outMsg;

	// send message
	sendDataToServer(hMsg, outMsg, 3);

	// add to outMsg carrete manipulating
	outMsg += std::string("\r\n\0");

	// clear window with message
	SetWindowText(hMsg, "");
	send_data = send(clientSocket, outMsg.c_str(), static_cast<int>(outMsg.size()), 0);
	/*outMsg = std::string("> ") + outMsg;
	if (send_data == SOCKET_ERROR)
	{
		std::string error = "Send failed" + std::string("\r\n");
		setText(hTextArea, error);
	}
	else
	{
		setText(hTextArea, outMsg);
	}*/

	return;
}

void sendDataToServer(HWND& hWnd, std::string& text, const int& addSize)
{
	// variables
	int length = 0;
	char* buffer{};

	// code
	length = GetWindowTextLength(hWnd) + addSize;
	buffer = new char[length];
	ZeroMemory(buffer, length * sizeof(char));
	GetWindowText(hWnd, buffer, length);
	text = buffer;

	// delete trash memory
	delete[] buffer;
}

// receive messeges from server
DWORD receiveMessages(LPVOID lParam)
{
	// Cast the parameter to the correct type
	HWND hMainWindow = reinterpret_cast<HWND>(lParam);

	int receive_data;
	char buffer[BF_SIZE];
	std::string message, errorMsg;

	while (true)
	{
		// Set receive_data to the result of recv, which will be -1 if no data is available
		ZeroMemory(buffer, sizeof(buffer));
		receive_data = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (receive_data == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS)
			{
				// No data available, continue loop
				continue;
			}
			else
			{
				// Error handling
				wndControl = false;
				closesocket(clientSocket);
				WSACleanup();

				// Set error to a text area
				errorMsg = "SERVER disconnected";
				setText(hTextArea, errorMsg);

				// Refresh main window
				InvalidateRect(hMainWindow, NULL, TRUE);
				UpdateWindow(hMainWindow);
				break;
			}
		}
		else if (receive_data == 0)
		{
			// Connection closed by server
			wndControl = false;
			closesocket(clientSocket);
			WSACleanup();

			// Refresh main window
			InvalidateRect(hMainWindow, NULL, TRUE);
			UpdateWindow(hMainWindow);
			break;
		}
		else
		{
			// Data received
			message = buffer;
			setText(hTextArea, message);
		}
	}

	return 0;
}

void setText(HWND& hWnd, std::string& text)
{
	//int textLength = GetWindowTextLength(hWnd);
	SendMessage(hWnd, EM_SETSEL, -1, -1);
	SendMessage(hWnd, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text.c_str()));
	text.clear();
}