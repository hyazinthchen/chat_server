//A simple server application using TCPv6-Sockets
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_FAMILY     AF_INET6     // Protocol family - in this case force IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM  // TCP uses SOCK_STREAM, UDP uses SOCK_DGRAM
#define DEFAULT_PORT       "1234"       // The port for testing
#define BUFFER_SIZE        1024         // The buffer size for the demonstration

int main(int argc, char* argv[]) {
	char buffer[BUFFER_SIZE];
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	int iResult, iSendResult;
	char recvbuf[BUFFER_SIZE];
	int recvbuflen = BUFFER_SIZE;

	// argc[0] = sNummer, argc[1] = portnumber where server listens argc < 2
	if (argc < 1) {
		fprintf(stderr, "Application needs sNumber and portnumber as arguments.\n");
		exit(1);
	}

	printf("\nServer has been started.\n");

	// Initialise TCP for Windows ("Winsock").
	WORD wVersionRequested; //WORD = unsigned long data type, version of the Winsocket
	WSADATA wsaData; // stores information about the Winsocket implementation
	wVersionRequested = MAKEWORD(2, 2); //MAKEWORD = macro to request version 2.2

	iResult = WSAStartup(wVersionRequested, &wsaData); //makes it possible for the process to run WINSOCK.DLL
	if (iResult != 0) {
		printf("\nError on initialising Winsock.\n");
		exit(1);
	}
	else {
		printf("\nWinsock initialised.\n");
	}

	ZeroMemory(&hints, sizeof(hints)); //Macro to fill a block of memory with zeros
	hints.ai_family = DEFAULT_FAMILY;
	hints.ai_socktype = DEFAULT_SOCKTYPE;
	hints.ai_protocol = IPPROTO_TCP; //specifies the TCP protocol
	hints.ai_flags = AI_PASSIVE; //flag to indicate that the returned socket address will be used in bind()

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	// Open a socket with the correct address family for this address.
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("\nError opening socket.\n");
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}
	else {
		printf("\nSocket has been opened.\n");
	}

	// bind() the server that will accept client connections to a network address
	// sockaddr holds information about the IP address and port number
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("\nError on binding socket.\n");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	else {
		printf("\nBinding successful.\n");
	}

	//listen() for incoming connections
	if (listen(ListenSocket, 5) == SOCKET_ERROR) { //5 = maximum length of the queue of pending connections to accept
		printf("\nError on listening.\n");
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	else {
		printf("\nListening on port %s.\n", DEFAULT_PORT);
	}

	//accept() a new connection
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("\nError on accept.\n");
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}


	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			printf("Message: %s\n", recvbuf);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	//close() the socket
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}