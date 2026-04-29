#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <functional>
#include "Nim.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

bool shouldLog = false;


int numPiles;
vector<int> piles;
bool gameOver = false;
bool turn;

bool CheckPiles();
void CreatePile(string board);
bool ValidMove(string move);
void UpdateBoard(string move);
void EndGame(char type = 'n');
bool isNDigits(string s, int numDigits);
void startGame(function<void(string)> send, function<string()> receive, bool isServer);

string msg;
string blank;

int host(string name);
int join(string name);

template<typename... Args>
void log(Args&&... args) {
	if (shouldLog) {
		cout << "[DEBUG]: ";
		((std::cout << args), ...);
		std::cout << '\n';
	}
}

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
		cout << endl;



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
				//cout << "recv() failed: " << WSAGetLastError() << endl;
				return "";
			}

			// filter to only receive from peer
			if (peer.sin_addr.S_un.S_addr == INADDR_ANY || // dont filter
				(incomingAddr.sin_addr.S_un.S_addr == peer.sin_addr.S_un.S_addr &&
				incomingAddr.sin_port == peer.sin_port)) {

				string s(recvbuf);
				log("Received '", s, "'");
				return s;
			}			
		}

		wait(s, 1, 0);
		secondsWaited++;
	}

	log("timeout");

	return "";
}

string receiveAndHandleMessage(function<string()> receive) {
	string received = receive();

	log("in handle '", received, "'");

	// timeout
	if (received.size() == 0) {
		EndGame('d');
	}
	// forfeit
	else if (tolower(received[0]) == 'f') {
		EndGame('f');
	}
	// chat
	else if (tolower(received[0]) == 'c' && received.size() > 1) {
		cout << "Opponent: " << received.substr(1);
		return receiveAndHandleMessage(receive);
	}
	// move/board
	else if (isdigit(received[0])) {
		if (ValidMove(received) || isNDigits(received, 7)) {
			return received;
		}
		else {
			EndGame('d');
		}
	}
	// invalid
	else {
		// message not in recognizeable format: win by default
		EndGame('d');
	}

	// invalid/empty...
	return "";
}

int send(SOCKET s, string msg, sockaddr_in peer) {

	log("sending '", msg, "'");
	
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
			//cout << "recv() failed: " << WSAGetLastError() << endl;
		}

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

			cout << "You have been challenged by " + peerName << '\n';
			while (response == "") {
				cout << "Do you accept (y/n): ";
				cin >> response;
				getline(cin, blank);
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
		iResult = send(s, sendbuf, peer);
		if (iResult) {
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
					return 1;
				}
			};
			auto boundReceive = [&]() {
				auto rec = [&]() {
					return receive(s, 30, peer);
				};

				return receiveAndHandleMessage(rec);
			};

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
			getline(cin, blank);
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


		iResult = send(s, NIM_CHALLENGE + name, servers[serverSelected].addr);
		if (iResult) {
			return 1;
		}


		// Receive response to challenge
		string response = receive(s, 10, servers[serverSelected].addr);

		// If they say no (anything non-yes)
		if (_stricmp(response.c_str(), "YES") != 0) {
			cout << servers[serverSelected].name << " did not accept your challenge.\n\n";
			continue;
		}

		// They said YES

		cout << servers[serverSelected].name << " accepted your challenge!\n\n";

		acceptedChallenge = true;

		// send "GREAT!"

		// BUG broke here
		iResult = send(s, NIM_CONFIRM, servers[serverSelected].addr);
		if (iResult) {
			return 1;
		}


		// game logic

		auto boundSend = [&](string msg) {
			int result = send(s, msg, servers[serverSelected].addr);
			if (result) {
				return 1;
			}
		};
		auto boundReceive = [&]() {
			auto rec = [&]() {
				return receive(s, 30, servers[serverSelected].addr);
			};

			return receiveAndHandleMessage(rec);
		};

		startGame(boundSend, boundReceive, false);

	}

	

	closesocket(s);
	WSACleanup();

	return -1;
}


// ************************************************************************************************
// ************************************************************************************************


bool isNDigits(string s, int numDigits) {
	if (s.length() != numDigits) return false;
	for (char c : s) {
		if (!isdigit(c)) return false;
	}
	return true;
}

void PrintBoard() {
	cout << "\nBoard:\n";
	for (int i : piles)
	{
		cout << i << " ";
	}
	cout << endl;
}

// puts move in msg (may be invalid)
void Prompt(function<void(string)> send) {
	int response = 0;

	while (response < 1 || response > 3) {
		cout << "1. Make move\n";
		cout << "2. Enter chat message\n";
		cout << "3. Forfeit game\n";
		cout << "What would you like to do?: ";
		cin >> response;
	}
	getline(cin, blank);
	cout << '\n';

	if (response == 1) {
		cout << "Enter move: ";
		getline(cin, msg);
	}
	else if (response == 2) {
		// send message
		cout << "Chat: ";
		getline(cin, msg);
		send("C" + msg + '\n');

		// get move
		Prompt(send);
	}
	else /* response == 3 */ {
		send("Forfeit match");
		EndGame('f');
	}
}

void startGame(function<void(string)> send, function<string()> receive, bool isServer) {
	numPiles = 0;
	piles = {};
	gameOver = false;


	if (isServer)
	{
		turn = false;
		bool validBoard = false;

		cout << "Enter the initial board\n";
		while (!validBoard)
		{
			getline(cin, msg);
			if (msg.size() == 0 || msg.size() - 1 != (msg.at(0) - '0') * 2)
			{
				cout << "Invalid Board, try again\n";
			}
			else
			{
				CreatePile(msg);
				if (!CheckPiles())
				{
					cout << "Invalid Board, try again\n";
				}
				else
				{
					validBoard = true;
				}
			}
		}

		send(msg);
	}
	else // client start
	{
		cout << "Waiting for opponent.\n";
		CreatePile(receive());
		if (!CheckPiles())
		{
			EndGame('d');
		}

		PrintBoard();

		turn = true;

		Prompt(send);
		if (gameOver) return;

		UpdateBoard(msg);
		send(msg);
		turn = false;
	}

	//PrintBoard();

	while (!gameOver)
	{
		cout << "\nWaiting for opponent.\n";

		msg = receive();
		if (gameOver) return;

		UpdateBoard(msg);
		if (gameOver) return;


		turn = true;

		Prompt(send);
		if (gameOver) return;

		UpdateBoard(msg);
		send(msg);
		turn = false;
	}
}


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

bool ValidMove(string move) {
	msg = move;
	int selectedPile = msg[0] - '0';
	int rocksRemove = 0;
	if (move[1] - '0' == 1)
	{
		rocksRemove += (msg[2] - '0') + 10;
	}
	else if (msg[1] - '0' == 2)
	{
		rocksRemove += (msg[2] - '0') + 20;
	}
	else if (msg[1] - '0' > 2)
	{
		rocksRemove += 21;
	}
	else
	{
		rocksRemove += msg[2] - '0';
	}

	if (!isNDigits(msg, 3), selectedPile > numPiles || selectedPile < 1 || piles.at(selectedPile - 1) == 0 || rocksRemove > piles.at(selectedPile - 1) || rocksRemove <= 0)
	{
		return false;
	}

	return true;
}

int parseRocks() {
	if (msg.size() < 3) return 0;
	int d = msg[1] - '0';
	if (d == 1)      return (msg[2] - '0') + 10;
	else if (d == 2) return (msg[2] - '0') + 20;
	else if (d > 2)  return 21;
	else             return msg[2] - '0';
}

std::vector<int> CreateMove(string move) {
	msg = move;
	int selectedPile = msg.size() >= 1 ? msg[0] - '0' : 0;
	int rocksRemove = parseRocks();

	while (!isNDigits(msg, 3) || selectedPile > numPiles || selectedPile < 1 || piles.at(selectedPile - 1) == 0 || rocksRemove > piles.at(selectedPile - 1) || rocksRemove <= 0)
	{
		if (!isNDigits(msg, 3)) {
			cout << "Enter 3 digits.\n";
		}
		else if (selectedPile > numPiles || selectedPile < 1) {
			cout << "Pile doesn't exist. Please try again.\n";
		}
		else if (piles.at(selectedPile - 1) == 0) {
			cout << "Empty Pile. Please try again.\n";
		}
		else if (rocksRemove > piles.at(selectedPile - 1) || rocksRemove <= 0) {
			cout << "Unable to remove " << rocksRemove << " rocks. Please try again.\n";
		}

		getline(cin, msg);
		selectedPile = msg.size() >= 1 ? msg[0] - '0' : 0;
		rocksRemove = parseRocks();
	}

	return { selectedPile, rocksRemove };
}

void UpdateBoard(string move)
{
	vector<int> result = CreateMove(move);

	int selectedPile = result[0];
	int rocksRemove = result[1];

	piles.at(selectedPile - 1) -= rocksRemove;

	bool empty = true;
	for (int i : piles)
	{
		if (i != 0) empty = false;
	}


	if (!turn) {
		cout << "Opponent's move: " << msg << '\n';
	}

	PrintBoard();

	if (empty)
	{
		EndGame();
	}
}


// n=normal, f=forfeit, d=default
void EndGame(char type)
{
	log("Type: ", type);

	if (type == 'd') cout << "You win by default\n\n\n";

	else if (type == 'f') {
		if (turn) cout << "You forfeit.\n\n\n";
		else cout << "Your opponent forfeited. You win!\n\n\n";
	}
	else /* type=='n' */ {
		if (turn) cout << "You win!\n\n\n";
		else cout << "You lose\n\n\n";
	}

	gameOver = true;
}