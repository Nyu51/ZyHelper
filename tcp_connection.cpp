#include <winsock2.h>
#include <iostream>
#include <tchar.h>

#include "tcp_connection.h"


#include "Debug.h"


using namespace std;



TCPConnection::TCPConnection()
{
}

int TCPConnection::Init()
{
	// Winsock starten
	WSADATA wsa;
	error_code = WSAStartup(MAKEWORD(2, 0), &wsa);
	if (error_code != 0)
	{
		MessageBox(NULL, _T("error couldnt run WSAStartup"),
			_T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
	DEBUG_MSG("WSAStartup" << endl);

	// Socket erstellen
	acceptSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (acceptSocket == INVALID_SOCKET)
	{
		MessageBox(NULL, _T("error couldnt create socket"),
			_T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
	DEBUG_MSG("accepted socket" << endl);

	// Socket binden
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2206);
	addr.sin_addr.s_addr = ADDR_ANY;

	error_code = ::bind(acceptSocket, (SOCKADDR*)&addr, sizeof(addr));

	if (error_code == SOCKET_ERROR)
	{
		MessageBox(NULL, _T("error couldnt bind"),
			_T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
	DEBUG_MSG("bound socket" << endl);
	return 0;

}

int TCPConnection::Listen()
{
	while (1)
	{
		// In den listen Modus
		error_code = listen(acceptSocket, 10);
		if (error_code == SOCKET_ERROR)
		{
			MessageBox(NULL, _T("Error couldnt switch to listening mode"),
				_T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
			return 1;
		}
		DEBUG_MSG("started listening" << endl);

		// Verbindung annehmen
		connectedSocket = accept(acceptSocket, NULL, NULL);
		if (connectedSocket == INVALID_SOCKET)
		{
			MessageBox(NULL, _T("FError couldnt accept connection"),
				_T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
			return 1;
		}
		DEBUG_MSG("accepted the connection" << endl);

		StartupTime = GetTickCount();

		// Daten austauschen
		while (error_code != SOCKET_ERROR)
		{
			error_code = recv(connectedSocket, text, 255, 0);

			tcpmutex.lock();
			content = text;
			tcpmutex.unlock();

			//cout <<"recv:" << content <<" " << error_code<< endl;
			if (error_code == 0)
			{
				MessageBox(NULL, _T("terminated"),
					_T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
				break;
				break;
			}
			if (error_code == SOCKET_ERROR) 
			{
				MessageBox(NULL, _T("error recieved a socket error"),
					_T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			text[error_code] = '\0';
			Sleep(1);
		}
		MessageBox(NULL, _T("Socket Error"),
			_T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
	}
}

int TCPConnection::Exit()
{
	closesocket(acceptSocket);
	closesocket(connectedSocket);
	WSACleanup();
	return 0;
}

bool TCPConnection::ElementAt(unsigned  i, unsigned j)
{
	string copy;
	bool result;
	//if (i < content.length()) result = content.at(i);
	tcpmutex.lock();
	if (content.length() >= j)
	{
		copy = content;
	}
	else 
	{
		tcpmutex.unlock();
		return false;
	}
	tcpmutex.unlock();


	result = (copy.at(j) & (1 << i))!=0;


	return result;
}

bool TCPConnection::IsReady()
{
	if (GetTickCount() - 5000 < StartupTime) return false;

	tcpmutex.lock();
	bool viable = content.length() >= 2;
	tcpmutex.unlock();

	if (viable)
	{
		return true;
	}
	return false;
}

TCPConnection::~TCPConnection()
{
}