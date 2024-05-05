#pragma once
#ifndef RCMESSENGER_H
#define RCMESSENGER_H

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <thread>
#include <windows.h>
#include <CommCtrl.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

// cases
#define CLIENT_CONNECT 1
#define SEND_MSG 2

// buffer size
#define BF_SIZE 1024

// network
extern WSAData wsData;
extern SOCKET clientSocket;
extern in_addr IPinAddr;
extern sockaddr_in serverSockaddr;
extern sockaddr_in clientSockaddr;
// main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");
// string that appears in the application's title bar.
static TCHAR szTitle[] = _T("RCMessenger");
// stored instance handle for use in Win32 API calls such as FindResource
extern HINSTANCE hInst;
// for thread
extern HANDLE threadMsg;
// for widgets
extern HWND ipHwnd;
extern HWND hTextArea;
extern HWND hMsg;
// font
extern HFONT font;
// flag to disable windows
extern bool wndControl;

bool createConnect(HWND& hWnd, const int& family, const int& type, const std::string& IPv4, const int& port, const int& flag);
void sendMessages();
void sendDataToServer(HWND& hWnd, std::string& text, const int& addSize);
DWORD receiveMessages(LPVOID lParam);
void setText(HWND& hWnd, std::string& text);

#endif