//A simple server application using TCPv6-Sockets
//sources:
//http://www.c-worker.ch/tuts/select.php
//https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include "Packet.h"

#define DEFAULT_FAMILY     AF_INET6     //Protocol family - in this case force IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM  //TCP uses SOCK_STREAM, UDP uses SOCK_DGRAM

//initialise TCP for Windows ("Winsock")
void initialiseWinsock() {
	int returnValue;

	WORD wVersionRequested; //WORD = unsigned long data type, version of the Winsocket
	WSADATA wsaData; //stores information about the Winsocket implementation
	wVersionRequested = MAKEWORD(2, 2); //MAKEWORD = macro to request version 2.2

	returnValue = WSAStartup(wVersionRequested, &wsaData); //makes it possible for the process to run WINSOCK.DLL
	if (returnValue != 0) {
		printf("Error on initialising Winsock.\n");
		exit(1);
	}
	else {
		printf("Winsock initialised.\n");
	}
}

int main(int argc, char* argv[]) {
	SOCKET serverSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, hints;
	int returnValue;
	FD_SET fdSet;
	struct Packet message_serv;
	struct Packet message_cli;

	if (argc < 3) {
		fprintf(stderr, "Application needs port and S-Number as arguments.\nUsage: Server.exe 1234 s12345\n");
		exit(1);
	}
	if (argc > 3) {
		fprintf(stderr, "You have entered too many arguments. This application only accepts 2 arguments. Anything else will be ignored.\n");
	}

	const char* port = argv[1];

	//initalise datastruct
	strcpy(message_serv.text, " ");
	strcpy(message_serv.name, argv[2]);

	printf("Server has been started.\n");

	initialiseWinsock();

	ZeroMemory(&hints, sizeof(hints)); //Macro to fill a block of memory with zeros
	hints.ai_family = DEFAULT_FAMILY;
	hints.ai_socktype = DEFAULT_SOCKTYPE;
	hints.ai_protocol = IPPROTO_TCP; //specifies the TCP protocol
	hints.ai_flags = AI_PASSIVE; //flag to indicate that the returned socket address will be used in bind()

	returnValue = getaddrinfo(NULL, port, &hints, &result);
	if (returnValue != 0) {
		printf("getaddrinfo failed: %d\n", returnValue);
		WSACleanup();
		exit(1);
	}

	//open a socket with the correct address family for this address
	serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (serverSocket == INVALID_SOCKET) {
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
	returnValue = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen);
	if (returnValue == SOCKET_ERROR) {
		printf("Error on binding socket.\n");
		freeaddrinfo(result);
		closesocket(serverSocket);
		WSACleanup();
		exit(1);
	}
	else {
		printf("Binding successful.\n");
	}

	//listen() for incoming connections
	if (listen(serverSocket, 5) == SOCKET_ERROR) { //5 = maximum length of the queue of pending connections to accept
		printf("\nError on listening.\n");
		closesocket(serverSocket);
		WSACleanup();
		exit(1);
	}
	else {
		printf("Listening on port %s.\n", port);
	}

	while (1) {

		FD_ZERO(&fdSet); //fdSet is an Array of Sockets - FD_Zero initalizes them with 0
		FD_SET(serverSocket, &fdSet); //add the socket which listens and awaits connections from client

		if (clientSocket != INVALID_SOCKET) {
			FD_SET(clientSocket, &fdSet); //add client socket too
		}

		//wait for something to happen on any of the sockets in fdSet, last parameter indicates the timeout (NULL means wait indefinitely)
		returnValue = select(0, &fdSet, NULL, NULL, NULL);
		if (returnValue == SOCKET_ERROR) {
			printf("Error on select.\n");
			exit(1);
		}

		//check, if serverSocket is in fdSet (fdSet is an Array of Sockets)
		if (FD_ISSET(serverSocket, &fdSet)) {
			if (clientSocket == INVALID_SOCKET) {
				clientSocket = accept(serverSocket, NULL, NULL);
				printf("New client connection accepted.\n");
			}
		}

		if (FD_ISSET(clientSocket, &fdSet)) {
			returnValue = recv(clientSocket, &message_cli, sizeof(message_cli), 0); //read the incoming message from the client

			//if recv returns 0, the client has closed the connection
			if (returnValue == 0 || returnValue == SOCKET_ERROR) {
				printf("Client %d has disconnected\n", clientSocket);
				closesocket(clientSocket); //close the socket   
				clientSocket = INVALID_SOCKET; //make room for new clients
			} else {
				printf("%s> %s\n", message_cli.name, message_cli.text);
				printf("%s> ", message_serv.name);
				gets(&message_serv.text);
				returnValue = send(clientSocket, &message_serv, sizeof(message_serv), 0);
				if (returnValue == SOCKET_ERROR) {
					printf("Send failed: %d\n", WSAGetLastError());
					closesocket(clientSocket);
					WSACleanup();
					exit(1);
				}
			}
		}
	}
}
		
	
	
