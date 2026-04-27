#pragma once

#define MAX_NAME         80 
#define MAX_SERVERS      100
#define DEFAULT_BUFLEN   512
#define DEFAULT_PORT     29333
#define NIM_QUERY        "Who?"
#define NIM_CHALLENGE    "Player="
#define NIM_CONFIRM      "GREAT!"
#define NIM_NAME	     "Name="


//#define Study_JOIN       "Join="
//#define Study_WHERE      "Where?"
//#define Study_LOC	     "Loc="
//#define Study_WHAT       "What?"
//#define Study_COURSES    "Courses="
//#define Study_MEMBERS    "Members?"
//#define Study_MEMLIST    "Members="

struct ServerStruct {
	char name[MAX_NAME];
	sockaddr_in addr;
};

int getServers(SOCKET s, ServerStruct server[]);
int wait(SOCKET s, int seconds, int msec);
sockaddr_in GetBroadcastAddress(char* IPAddress, char* subnetMask);
sockaddr_in GetBroadcastAddressAlternate(char* IPAddress, char* subnetMask);