#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Comctl32.lib")
#include "rcmessenger/rcmessenger.h"
// for icon
#include "../resource.h"

//Connection parameters
constexpr const int port = 55555;
constexpr const int family = AF_INET;
constexpr const int type = SOCK_STREAM;
constexpr const int flag = 0;
std::string IP;

WNDCLASSEX windowParams(HBRUSH BGColor, HCURSOR Cursor, HINSTANCE hInst, HICON Icon, LPCSTR Name, WNDPROC Procedure)
{
	WNDCLASSEX wnd = { 0 };

	wnd.hbrBackground = BGColor;
	wnd.hCursor = Cursor;
	wnd.hInstance = hInst;
	wnd.hIcon = Icon;
	wnd.lpszClassName = Name;
	wnd.lpfnWndProc = Procedure;

	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.lpszMenuName = NULL;
	wnd.hIconSm = LoadIcon(wnd.hInstance, IDI_APPLICATION);

	return wnd;
}

void widgets(HWND hWnd)
{
	//greetings
	 CreateWindowA("static", "RCMessenger 1.0", WS_VISIBLE | WS_CHILD, 10, 22, 250, 20, hWnd, NULL, NULL, NULL);

	//IP
	CreateWindowA("static", "IP:", WS_VISIBLE | WS_CHILD, 280, 22, 20, 20, hWnd, NULL, NULL, NULL);
	ipHwnd = CreateWindowA("edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 305, 20, 110, 20, hWnd, NULL, NULL, NULL);

	//connect
	CreateWindowA("button", "Connect", WS_VISIBLE | WS_CHILD, 425, 20, 85, 20, hWnd, (HMENU)CLIENT_CONNECT, NULL, NULL);

	//text area
	hTextArea = CreateWindowA(
		"edit", "", 
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
		10, 60, 500, 300, hWnd, 
		NULL, NULL, NULL
	);

	//mesage area
	hMsg = CreateWindowA("edit", "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, 10, 370, 405, 20, hWnd, NULL, NULL, NULL);

	//send button
	CreateWindowA("button", "Send", WS_VISIBLE | WS_CHILD, 425, 370, 85, 20, hWnd, (HMENU)SEND_MSG, NULL, NULL);
}

LRESULT CALLBACK sendEnt(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (message)
	{
	case WM_CHAR:
		switch (wParam)
		{
		case VK_RETURN:
			sendMessages();
			return 0;
		}
	}
	return DefSubclassProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	EnableWindow(ipHwnd, (!wndControl));
	EnableWindow(GetDlgItem(hWnd, CLIENT_CONNECT), (!wndControl));
	EnableWindow(hMsg, wndControl);
	EnableWindow(GetDlgItem(hWnd, SEND_MSG), wndControl);

	switch (message)
	{
	case WM_CREATE:
		widgets(hWnd);
		SendMessageA(hWnd, WM_SETFONT, (WPARAM)font, TRUE);
		SetWindowSubclass(hMsg, sendEnt, 0, 0);
		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case CLIENT_CONNECT:

			//take IP
			sendDataToServer(ipHwnd, IP, 1);

			//if connect is success, create thread to receive messages
			if (createConnect(hTextArea, family, type, IP, port, flag))
			{
				wndControl = true;
				threadMsg = CreateThread(NULL, 0, receiveMessages, NULL, 0, NULL);
			}
			break;
		case SEND_MSG:
			sendMessages();
			break;
		default: break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
)
{
	font = CreateFontA(
		17, 7, 0, 0, FW_SEMIBOLD,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		FF_SWISS, "SwissSemibold"
	);

	WNDCLASSEX windowsClass = windowParams(
		(HBRUSH)COLOR_WINDOW,
		LoadCursor(NULL, IDC_ARROW),
		hInstance,
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
		szWindowClass,
		WndProc
	);

	//registration of window class
	// _T is for tchar
	if (!RegisterClassEx(&windowsClass)) // this is WNDCLASSEX
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			szWindowClass,
			NULL);

		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;


	/*Creat window function*/
	// The parameters to CreateWindowEx explained:
	// WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		550, 450,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindowEx failed!"),
			szWindowClass,
			NULL);

		return 1;
	}

	/*make window visible*/
	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//message loop
	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}