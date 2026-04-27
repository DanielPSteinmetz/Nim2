//#include "Nim.h"
//#include <iostream>
//#include <WS2tcpip.h>
//#include <string>
//
//
//#pragma comment(lib, "Ws2_32.lib")
//
//using namespace std;
//
//int host(string name);
//int join(string name);
//
////int main() {
//	//string name;
////	int userChoice = 0;
////	cout << "What is your name?: ";
////	getline(cin, name);
////
////	int status = 0;
////
////	while (userChoice < 1 || userChoice > 3 || status == -1) {
////		cout << "What would you like to do? (enter number of choice)\n";
////		cout << "1. Host Nim game\n";
////		cout << "2. Join Nim game\n";
////		cout << "3. Exit\n";
////		cout << "Response: ";
////		cin >> userChoice;
////
////		string blank;
////		getline(cin, blank);
////
////
////		if (userChoice == 1) {
////			status = host(name);
////		}
////		else if (userChoice == 2) {
////			status = join(name);
////		}
////		else if (userChoice == 3) {
////			status = 0;
////		}
////	}
////
////	return status;
////}
//
//int host(string name) {
//	// *** Init Winsock ***
//	WSADATA wsaData;
//
//	int iResult;
//
//	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//	if (iResult != 0) {
//		cout << "WSAStartup() failed: " << iResult << endl;
//		return 1;
//	}
//
//	// *** Creating a socket ***
//	SOCKET s = INVALID_SOCKET;
//	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//	if (s == INVALID_SOCKET) {
//		cout << "socket() failed: " << WSAGetLastError() << endl;
//
//		WSACleanup();
//		return 1;
//	}
//
//	// BIND SOCKET
//	sockaddr_in service;
//	service.sin_family = AF_INET;
//	service.sin_addr.s_addr = INADDR_ANY;
//	service.sin_port = htons(DEFAULT_PORT);
//
//	bind(s, (SOCKADDR*)&service, sizeof(service));
//
//
//	// INIT BUFFERS
//	int recvbuflen = DEFAULT_BUFLEN;
//	char recvbuf[DEFAULT_BUFLEN];
//	char sendbuf[DEFAULT_BUFLEN];
//
//
//	sockaddr_in senderAddr;
//	int senderAddrSize = sizeof(senderAddr);
//
//
//	bool acceptChallenge = false;
//
//	while (true) {
//		while (wait(s, 1, 0) == 0) {}
//
//		iResult = recvfrom(s, recvbuf, recvbuflen, 0, (SOCKADDR*)&senderAddr, &senderAddrSize);
//		if (iResult < 0) {
//			cout << "recv() failed: " << WSAGetLastError() << endl;
//		}
//
//		cout << "Received (debug): " << recvbuf << '\n'; // output cmd
//
//		if (_stricmp(recvbuf, NIM_QUERY) == 0) {
//			strcpy_s(sendbuf, DEFAULT_BUFLEN, NIM_NAME);
//			strcat_s(sendbuf, DEFAULT_BUFLEN, name.c_str());
//		}
//		else if (_strnicmp(recvbuf, NIM_CHALLENGE, 7) == 0) {
//			string name(recvbuf);
//			name = name.substr(7); //name of player challenging us (Player=____)
//			string response = "";
//
//			cout << "You have been challenged by " + name << '\n';
//			while (response == "") {
//				cout << "Do you accept (y/n): ";
//				cin >> response;
//			}
//
//			if (tolower(response[0]) == 'y') {
//				strcpy_s(sendbuf, DEFAULT_BUFLEN, "YES");
//				acceptChallenge = true;
//			}
//			else {
//				strcpy_s(sendbuf, DEFAULT_BUFLEN, "NO");
//				acceptChallenge = false;
//			}
//
//
//			// look for "GREAT!" if yes: play, if no: listen
//
//		}
//		else {
//			strcpy_s(sendbuf, DEFAULT_BUFLEN, "Unrecognized Command");
//		}
//
//
//		// send command
//		iResult = sendto(s, sendbuf, (int)strlen(sendbuf) + 1, 0, (SOCKADDR*)&senderAddr, sizeof(senderAddr));
//		if (iResult == SOCKET_ERROR) {
//			cout << "send() failed: " << WSAGetLastError() << endl;
//			closesocket(s);
//			//freeaddrinfo(result);
//			WSACleanup();
//			return 1;
//		}
//	}
//
//	closesocket(s);
//	WSACleanup();
//
//	return 0;
//}
//
//
//// ************************************************************************************************
//
//
//int join(string name) {
//
//	// *** Init Winsock ***
//	WSADATA wsaData;
//
//	int iResult;
//
//	// needs to be closed before exit
//	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//	if (iResult != 0) {
//		cout << "WSAStartup() failed: " << iResult << endl;
//		return 1;
//	}
//
//	// *** Creating a socket ***
//	SOCKET s = INVALID_SOCKET;
//	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//	if (s == INVALID_SOCKET) {
//		cout << "socket() failed: " << WSAGetLastError() << endl;
//
//		WSACleanup();
//		return 1;
//	}
//
//	// buffers init
//	int recvbuflen = DEFAULT_BUFLEN;
//	char recvbuf[DEFAULT_BUFLEN];
//	char sendbuf[DEFAULT_BUFLEN];
//
//
//	ServerStruct servers[MAX_SERVERS];
//
//
//	bool acceptedChallenge = false;
//
//	// continue this step until they quit or have their challenge accepted
//	while (!acceptedChallenge) {
//
//		// Display servers
//		int numServers = getServers(s, servers);
//		if (numServers == 0) {
//			cout << "No one available to join.\n";
//			closesocket(s);
//			WSACleanup();
//			return -1;
//		}
//
//		for (int i = 0; i < numServers; ++i) {
//			cout << i + 1 << ". " << servers[i].name << '\n';
//		}
//		cout << numServers + 1 << ". Exit Program" << '\n';
//
//
//		// Challenge player / quit
//		int serverSelected = 0;
//		while (serverSelected < 1 || serverSelected > numServers + 1) {
//			cout << "Who would you like to challenge? (" << numServers + 1 << " to exit): ";
//			cin >> serverSelected;
//		}
//
//		if (serverSelected == numServers + 1) {
//			cout << "Bye\n";
//			closesocket(s);
//			WSACleanup();
//			return 0;
//		}
//
//		cout << "Challenge sent to " << serverSelected << ".\n";
//
//		serverSelected -= 1;
//
//
//		// Send challenge
//		sockaddr_in senderAddr;
//		int senderAddrSize = sizeof(senderAddr);
//
//		strcpy_s(sendbuf, DEFAULT_BUFLEN, NIM_CHALLENGE);
//		strcat_s(sendbuf, DEFAULT_BUFLEN, name.c_str());
//
//		iResult = sendto(s, sendbuf, (int)strlen(sendbuf) + 1, 0, (SOCKADDR*)&servers[serverSelected].addr, sizeof(servers[serverSelected].addr));
//		if (iResult == SOCKET_ERROR) {
//			cout << "send() failed: " << WSAGetLastError() << endl;
//			closesocket(s);
//			WSACleanup();
//			return 1;
//		}
//
//
//		// Receive response to challenge
//		wait(s, 10, 0); // wait 10 sec for YES/NO
//		iResult = recvfrom(s, recvbuf, recvbuflen, 0, (SOCKADDR*)&senderAddr, &senderAddrSize);
//		if (iResult < 0) {
//			cout << "recv() failed: " << WSAGetLastError() << endl;
//		}
//
//		// Filter only our server
//		if (senderAddr.sin_addr.S_un.S_addr == servers[serverSelected].addr.sin_addr.S_un.S_addr &&
//			senderAddr.sin_port == servers[serverSelected].addr.sin_port) {
//			cout << recvbuf << '\n';
//		}
//		else {
//			// If we didn't receive from our server, its the same as "NO"
//			continue;
//		}
//
//		// If they say no (anything non-yes)
//		if (_stricmp(recvbuf, "YES") != 0) {
//			closesocket(s);
//			WSACleanup();
//			return 0;
//		}
//
//		strcpy_s(sendbuf, DEFAULT_BUFLEN, NIM_CONFIRM);
//
//		iResult = sendto(s, sendbuf, (int)strlen(sendbuf) + 1, 0, (SOCKADDR*)&servers[serverSelected].addr, sizeof(servers[serverSelected].addr));
//		if (iResult == SOCKET_ERROR) {
//			cout << "send() failed: " << WSAGetLastError() << endl;
//			closesocket(s);
//			WSACleanup();
//			return 1;
//		}
//	}
//
//	closesocket(s);
//	WSACleanup();
//
//	return 0;
//}