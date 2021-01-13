//A simple server application using TCPv6-Sockets
// sources:
//http://www.c-worker.ch/tuts/select.php
//https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_FAMILY     AF_INET6     //Protocol family - in this case force IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM  //TCP uses SOCK_STREAM, UDP uses SOCK_DGRAM
#define BUFFER_SIZE        1024         //The buffer size for the demonstration
#define MAX_CLIENTS		   10

//data structure that will be sent to the client
struct package {
	char sNumber[7];
	char txt[BUFFER_SIZE];
};

char* init_package(struct package p) {
	char* package_ZK = NULL;
	int sNumberLength = strlen(p.sNumber);
	int txtLength = strlen(p.txt);
	int totalLength = sNumberLength + 2 + txtLength - 1;
	char spacer[2] = { ':', ' ' };

	package_ZK = (char*)malloc(totalLength * sizeof(char)); //allocate memory for the package

	if (package_ZK == NULL) {
		printf("Error on building package.\n");
		EXIT_FAILURE;
	}

	int i = 0;
	int j = 0;

	for (i; i < sNumberLength; i++) {
		package_ZK[i] = p.sNumber[i];
	}

	package_ZK[i] = spacer[0];
	i++;
	package_ZK[i] = spacer[1];
	i++;

	for (j = 0, i; j <= txtLength; i++, j++) {
		package_ZK[i] = p.txt[j];
	}

	return package_ZK;
}

int main(int argc, char* argv[]) {
	char buffer[BUFFER_SIZE];
	SOCKET ListenSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	int iResult, iSendResult;
	char receiveBuffer[BUFFER_SIZE];
	int bufferLength = BUFFER_SIZE;
	FD_SET fdSet;
	SOCKET clients[MAX_CLIENTS];
	int i;

	if (argc < 2) {
		fprintf(stderr, "Application needs port and S-Number as arguments.\n");
		exit(1);
	}

	const char* port = argv[1];
	const char* sNumber = argv[2];

	printf("Server has been started.\n");

	//initialise TCP for Windows ("Winsock")
	WORD wVersionRequested; //WORD = unsigned long data type, version of the Winsocket
	WSADATA wsaData; //stores information about the Winsocket implementation
	wVersionRequested = MAKEWORD(2, 2); //MAKEWORD = macro to request version 2.2

	iResult = WSAStartup(wVersionRequested, &wsaData); //makes it possible for the process to run WINSOCK.DLL
	if (iResult != 0) {
		printf("Error on initialising Winsock.\n");
		exit(1);
	}
	else {
		printf("Winsock initialised.\n");
	}

	ZeroMemory(&hints, sizeof(hints)); //Macro to fill a block of memory with zeros
	hints.ai_family = DEFAULT_FAMILY;
	hints.ai_socktype = DEFAULT_SOCKTYPE;
	hints.ai_protocol = IPPROTO_TCP; //specifies the TCP protocol
	hints.ai_flags = AI_PASSIVE; //flag to indicate that the returned socket address will be used in bind()

	iResult = getaddrinfo(NULL, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	//open a socket with the correct address family for this address
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error opening socket.\n");
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}
	else {
		printf("Socket has been opened.\n");
	}

	//bind() the server that will accept client connections to a network address
	//sockaddr holds information about the IP address and port number
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("Error on binding socket.\n");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	else {
		printf("Binding successful.\n");
	}

	//listen() for incoming connections
	if (listen(ListenSocket, 10) == SOCKET_ERROR) { //10 = maximum length of the queue of pending connections to accept
		printf("\nError on listening.\n");
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	else {
		printf("Listening on port %s.\n", port);
	}



	//set all possible clients to INVALID_SOCKET
	for (i = 0;i < MAX_CLIENTS;i++) {
		clients[i] = INVALID_SOCKET;
	}

	while (1) {
		FD_ZERO(&fdSet); //Initializes the file descriptor set fdset to have zero bits for all file descriptors - empty the set
		FD_SET(ListenSocket, &fdSet); //add the socket which listens and awaits connections from clients

		// put all other available sockets that are not an INVALID_SOCKET into the set too, because we want to know which clients want to connect or send messages 
		for (i = 0;i < MAX_CLIENTS;i++) {
			if (clients[i] != INVALID_SOCKET) {
				FD_SET(clients[i], &fdSet);
			}
		}

		//wait for something to happen on any of the sockets in fdSet, last parameter indicates the timeout (NULL means wait indefinitely)
		iResult = select(0, &fdSet, NULL, NULL, NULL);
		if (iResult == SOCKET_ERROR) {
			printf("Error on select.\n");
			exit(1);
		}

		//when something happens on the socket the server is listening on it means a client wants to connect
		if (FD_ISSET(ListenSocket, &fdSet)) {
			//look for a place to store the new client in 
			for (i = 0; i < MAX_CLIENTS; i++) {
				if (clients[i] == INVALID_SOCKET) {
					clients[i] = accept(ListenSocket, NULL, NULL);
					printf("New client connection accepted.\n");
					break;
				}
			}
		}

		//when something happens on any other socket other than the ListenSocket, it means an I/O operation is happening there
		//check which other active sockets are in the set
		for (i = 0; i < MAX_CLIENTS; i++) {
			if (FD_ISSET(clients[i], &fdSet)) { //skip all sockets that have no client connected to them, only check those with an active connection
				iResult = recv(clients[i], receiveBuffer, 1024, 0); //read the incoming message from the client
				//if recv returns 0, the client has closed the connection
				if (iResult == 0 || iResult == SOCKET_ERROR) {
					printf("Client %d has disconnected\n", i);
					closesocket(clients[i]); //close the socket   
					clients[i] = INVALID_SOCKET; //make room for new clients
				}
				else {
					//add null character to printf message in buffer
					receiveBuffer[iResult] = '\0';
					printf("%s\n", receiveBuffer);

					gets(buffer);

					struct package p; //build new data package
					strcpy(p.sNumber, sNumber); //fill data package with data
					strcpy(p.txt, buffer);

					char* package_ZK = NULL;
					package_ZK = init_package(p); //pointer to the built data package

					iResult = send(clients[i], package_ZK, (int)strlen(package_ZK), 0);
					if (iResult == SOCKET_ERROR) {
						printf("Send failed: %d\n", WSAGetLastError());
						closesocket(clients[i]);
						WSACleanup();
						exit(1);
					}
				}
			}
		}
	}
}