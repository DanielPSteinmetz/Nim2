#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <functional>
#include "Nim.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int numPiles;
vector<int> piles;

bool isServer{ true };
bool gameOver = false;
bool turn;
bool CheckPiles();
void CreatePile(string board);
void UpdateBoard(string move);
void EndGame(bool def = false);

string msg;
string blank;

int host(string name);
int join(string name);

int main() {
	string name;
	int userChoice = 0;
	cout << "What is your name?: ";
	getline(cin, name);

	int status = 0;

	while (userChoice < 1 || userChoice > 3 || status == -1) {
		cout << "What would you like to do? (enter number of choice)\n";
		cout << "1. Host Nim game\n";
		cout << "2. Join Nim game\n";
		cout << "3. Exit\n";
		cout << "Response: ";
		cin >> userChoice;

		getline(cin, blank);


		if (userChoice == 1) {
			status = host(name);
		}
		else if (userChoice == 2) {
			status = join(name);
		}
		else if (userChoice == 3) {
			status = 0;
		}
	}

	return status;
}


string receive(SOCKET s, int secondsToWait = 2, sockaddr_in peer = {}) {

	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
		
	int secondsWaited = 0;
	while (secondsWaited < secondsToWait) {
		// go through everything waiting in buffer
		while (wait(s, 0, 0)) {

			sockaddr_in incomingAddr;
			int incomingAddrSize = sizeof(incomingAddr);

			int iResult = recvfrom(s, recvbuf, recvbuflen, 0, (SOCKADDR*)&incomingAddr, &incomingAddrSize);
			if (iResult < 0) {
				cout << "recv() failed: " << WSAGetLastError() << endl;
			}

			// filter to only receive from peer
			if (peer.sin_addr.S_un.S_addr == INADDR_ANY || // dont filter
				(incomingAddr.sin_addr.S_un.S_addr == peer.sin_addr.S_un.S_addr &&
				incomingAddr.sin_port == peer.sin_port)) {

				return string(recvbuf);
			}			
		}

		wait(s, 1, 0);
		secondsWaited++;
	}

	return "";
}

int send(SOCKET s, string msg, sockaddr_in peer) {
	
	char sendbuf[DEFAULT_BUFLEN];
	strcpy_s(sendbuf, DEFAULT_BUFLEN, msg.c_str());

	int iResult = sendto(s, sendbuf, (int)strlen(sendbuf) + 1, 0, (SOCKADDR*)&peer, sizeof(peer));
	if (iResult == SOCKET_ERROR) {
		cout << "send() failed: " << WSAGetLastError() << endl;
		closesocket(s);
		WSACleanup();
		return 1;
	}
	return 0;
}

void startGame(function<void(string)> send, function<string()> receive, bool isServer) {
	cout << "start game\n";
	if (isServer)
	{
		cout << "am server\n";

		turn = false;
		bool validBoard = false;
		while (!validBoard)
		{
			cout << "in !validBoard loop\n";


			getline(cin, msg);
			if (msg.size() == 0 || msg.size() - 1 != (msg.at(0) - '0') * 2)
			{
				cout << "Invalid Board\n";
			}
			else
			{
				CreatePile(msg);
				if (!CheckPiles())
				{
					cout << "Invalid Board\n";
				}
				else
				{
					validBoard = true;
				}
			}
		}
		cout << "bouta send\n";

		send(msg);
		cout << "sent\n";

	}
	else
	{
		CreatePile(receive());
		if (!CheckPiles())
		{
			EndGame(true);
		}
		turn = true;
		getline(cin, msg);
		UpdateBoard(msg);
		send(msg);
		turn = false;
	}

	cout << numPiles << endl;
	for (int i : piles)
	{
		cout << i << " ";
	}

	cout << endl;

	while (!gameOver)
	{
		msg = receive();
		UpdateBoard(msg);
		if (gameOver) break;
		turn = true;
		getline(cin, blank);
		getline(cin, msg);
		UpdateBoard(msg);
		send(msg);
		turn = false;
	}
}


int host(string name) {
	// *** Init Winsock ***
	WSADATA wsaData;

	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup() failed: " << iResult << endl;
		return 1;
	}

	// *** Creating a socket ***
	SOCKET s = INVALID_SOCKET;
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		cout << "socket() failed: " << WSAGetLastError() << endl;

		WSACleanup();
		return 1;
	}

	// BIND SOCKET
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(DEFAULT_PORT);

	::bind(s, (SOCKADDR*)&service, sizeof(service));


	// INIT BUFFERS
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	char sendbuf[DEFAULT_BUFLEN];


	sockaddr_in senderAddr;
	int senderAddrSize = sizeof(senderAddr);


	bool acceptChallenge = false;

	cout << "Waiting for someone to send a challenge\n";
	while (true) {
		// wait until something is in the socket
		while (wait(s, 1, 0) == 0) {}
		
		iResult = recvfrom(s, recvbuf, recvbuflen, 0, (SOCKADDR*)&senderAddr, &senderAddrSize);
		if (iResult < 0) {
			cout << "recv() failed: " << WSAGetLastError() << endl;
		}

		cout << "Received (debug): " << recvbuf << '\n'; // output cmd

		// save the address that sent to us
		sockaddr_in peer = senderAddr;
		string peerName;


		if (_stricmp(recvbuf, NIM_QUERY) == 0) {
			strcpy_s(sendbuf, DEFAULT_BUFLEN, NIM_NAME);
			strcat_s(sendbuf, DEFAULT_BUFLEN, name.c_str());
		}
		else if (_strnicmp(recvbuf, NIM_CHALLENGE, 7) == 0) {
			peerName = string(recvbuf);
			peerName = peerName.substr(7); //name of player challenging us (Player=____)
			string response = "";

			cout << "You have been challenged by " + name << '\n';
			while (response == "") {
				cout << "Do you accept (y/n): ";
				cin >> response;
			}

			if (tolower(response[0]) == 'y') {
				strcpy_s(sendbuf, DEFAULT_BUFLEN, "YES");
				acceptChallenge = true;
			}
			else {
				strcpy_s(sendbuf, DEFAULT_BUFLEN, "NO");
				acceptChallenge = false;
			}
		}
		else {
			strcpy_s(sendbuf, DEFAULT_BUFLEN, "Unrecognized Command");
		}


		// send command
		iResult = sendto(s, sendbuf, (int)strlen(sendbuf) + 1, 0, (SOCKADDR*)&senderAddr, sizeof(senderAddr));
		if (iResult == SOCKET_ERROR) {
			cout << "send() failed: " << WSAGetLastError() << endl;
			closesocket(s);
			WSACleanup();
			return 1;
		}


		// look for confirm after challenge
		if (acceptChallenge) {

			// look for "GREAT"
			string confirm = receive(s, 2, peer);
				
			// If didn't receive "GREAT", continue listening
			if (confirm != NIM_CONFIRM) {
				cout << peerName << " disconnected unexpectedly.\n";
				continue;
			}


			// bind send & receive and start game
			auto boundSend = [&](string msg) {
				int result = send(s, msg, peer);
				if (result) {
					cout << "send() failed: " << WSAGetLastError() << endl;
					closesocket(s);
					WSACleanup();
					return 1;
				}
			};
			auto boundReceive = [&]() {
				return receive(s, 200, peer); // BUG
			};

			cout << "debug: starging game";
			getline(cin, blank);
			if (blank != "") {
				cout << "Oops, shouldn't have put " << blank << " in blank\n";
			}
			startGame(boundSend, boundReceive, true);
			break;
		}
	}

	closesocket(s);
	WSACleanup();

	return -1;
}


// ************************************************************************************************


int join(string name) {

	// *** Init Winsock ***
	WSADATA wsaData;

	int iResult;

	// needs to be closed before exit
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup() failed: " << iResult << endl;
		return 1;
	}

	// *** Creating a socket ***
	SOCKET s = INVALID_SOCKET;
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		cout << "socket() failed: " << WSAGetLastError() << endl;

		WSACleanup();
		return 1;
	}

	// buffers init
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	char sendbuf[DEFAULT_BUFLEN];


	ServerStruct servers[MAX_SERVERS];


	bool acceptedChallenge = false;

	// continue this step until they quit or have their challenge accepted
	while (!acceptedChallenge) {

		// Display servers
		int numServers = getServers(s, servers);
		if (numServers == 0) {
			cout << "No one available to join.\n";
			closesocket(s);
			WSACleanup();
			return -1;
		}

		for (int i = 0; i < numServers; ++i) {
			cout << i + 1 << ". " << servers[i].name << '\n';
		}
		cout << numServers + 1 << ". Exit Program" << '\n';


		// Challenge player / quit
		int serverSelected = 0;
		while (serverSelected < 1 || serverSelected > numServers + 1) {
			cout << "Who would you like to challenge? (" << numServers + 1 << " to exit): ";
			cin >> serverSelected;
		}

		if (serverSelected == numServers + 1) {
			cout << "Bye\n";
			closesocket(s);
			WSACleanup();
			return -1;
		}
		serverSelected -= 1;

		cout << "Challenge sent to " << servers[serverSelected].name << ".\n";



		// Send challenge
		sockaddr_in senderAddr;
		int senderAddrSize = sizeof(senderAddr);

		strcpy_s(sendbuf, DEFAULT_BUFLEN, NIM_CHALLENGE);
		strcat_s(sendbuf, DEFAULT_BUFLEN, name.c_str());

		iResult = sendto(s, sendbuf, (int)strlen(sendbuf) + 1, 0, (SOCKADDR*)&servers[serverSelected].addr, sizeof(servers[serverSelected].addr));
		if (iResult == SOCKET_ERROR) {
			cout << "send() failed: " << WSAGetLastError() << endl;
			closesocket(s);
			WSACleanup();
			return 1;
		}


		// Receive response to challenge
		string response = receive(s, 10, servers[serverSelected].addr);

		// If they say no (anything non-yes)
		if (_stricmp(response.c_str(), "YES") != 0) {
			cout << servers[serverSelected].name << " did not accept your challenge.\n";
			continue;
		}

		// They said YES

		cout << servers[serverSelected].name << " accepted your challenge!";

		acceptedChallenge = true;

		// send "GREAT!"

		// BUG broke here
		iResult = send(s, NIM_CONFIRM, servers[serverSelected].addr);
		if (iResult == SOCKET_ERROR) {
			cout << "send() failed: " << WSAGetLastError() << endl;
			closesocket(s);
			WSACleanup();
			return 1;
		}


		// game logic

		auto boundSend = [&](string msg) {
			int result = send(s, msg, servers[serverSelected].addr);
			if (result) {
				cout << "send() failed: " << WSAGetLastError() << endl;
				closesocket(s);
				WSACleanup();
				return 1;
			}
			};
		auto boundReceive = [&]() {
			return receive(s, 200, servers[serverSelected].addr);
			};

		getline(cin, blank);
		if (blank != "") {
			cout << "Oops, shouldn't have put " << blank << " in blank\n";
		}
		startGame(boundSend, boundReceive, false);

	}

	

	closesocket(s);
	WSACleanup();

	return -1;
}


// ************************************************************************************************
// ************************************************************************************************


bool CheckPiles()
{
	if (numPiles < 3 || numPiles > 9) return false;

	for (int i = 0; i < numPiles - 1; i++)
	{
		if (piles.at(i) < 1 || piles.at(i) > 20) return false;
	}

	return true;
}

void CreatePile(string board)
{
	numPiles = board[0] - '0';
	piles.resize(numPiles);

	int pilesIndex = 0;
	for (int i = 1; i <= numPiles * 2; i += 2)
	{
		if (board[i] - '0' == 1)
		{
			piles.at(pilesIndex) += (board[i + 1] - '0') + 10;
		}
		else if (board[i] - '0' == 2)
		{
			piles.at(pilesIndex) += (board[i + 1] - '0') + 20;
		}
		else if (board[i] - '0' > 2)
		{
			piles.at(pilesIndex) += 21;
		}
		else
		{
			piles.at(pilesIndex) += board[i + 1] - '0';
		}
		pilesIndex++;
	}
}

void UpdateBoard(string move)
{
	int selectedPile = move[0] - '0';

	while (selectedPile - 1 >= numPiles || selectedPile - 1 < 0)
	{
		cout << "Pile doesn't exist. Choose another pile.\n";
		getline(cin, msg);
		while (msg.size() != 1)
		{
			cout << "Please enter a single digit.\n";
			getline(cin, msg);
		}
		selectedPile = msg[0] - '0';
	}

	while (piles.at(selectedPile - 1) == 0)
	{
		cout << "Empty Pile. Choose another pile.\n";
		getline(cin, msg);
		while (msg.size() != 1)
		{
			cout << "Please enter a single digit.\n";
			getline(cin, msg);
		}
		selectedPile = msg[0] - '0';
	}

	int rocksRemove = 0;
	if (move[1] - '0' == 1)
	{
		rocksRemove += (move[2] - '0') + 10;
	}
	else if (move[1] - '0' == 2)
	{
		rocksRemove += (move[2] - '0') + 20;
	}
	else if (move[1] - '0' > 2)
	{
		rocksRemove += 21;
	}
	else
	{
		rocksRemove += move[2] - '0';
	}

	while (rocksRemove > piles.at(selectedPile - 1) || rocksRemove <= 0)
	{
		cout << "Unable to remove " << rocksRemove << " rocks. Choose another amount.\n";
		getline(cin, msg);
		while (msg.size() != 2)
		{
			cout << "Please enter two digits.\n";
			getline(cin, msg);
		}
		rocksRemove = 0;
		if (msg[0] - '0' == 1)
		{
			rocksRemove += (msg[1] - '0') + 10;
		}
		else if (msg[0] - '0' == 2)
		{
			rocksRemove += (msg[1] - '0') + 20;
		}
		else if (msg[0] - '0' > 2)
		{
			rocksRemove += 21;
		}
		else
		{
			rocksRemove += msg[1] - '0';
		}
	}

	piles.at(selectedPile - 1) -= rocksRemove;

	bool empty = true;
	for (int i : piles)
	{
		if (i != 0) empty = false;
	}

	for (int i : piles)
	{
		cout << i << " ";
	}

	cout << endl;

	if (empty)
	{
		EndGame();
	}
}

void EndGame(bool def)
{
	if (def) cout << "You win by default\n";

	if (turn) cout << "You win!\n";
	else cout << "You lose\n";
	gameOver = true;
}