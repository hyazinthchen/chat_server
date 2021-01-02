//A simple server application using TCPv6-Sockets
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <Winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_FAMILY     AF_INET6     // Protocol family - in this case force IPv6
#define DEFAULT_SOCKTYPE   SOCK_STREAM  // TCP uses SOCK_STREAM, UDP uses SOCK_DGRAM
#define DEFAULT_PORT       1234         // The port for testing
#define BUFFER_SIZE        1024         // The buffer size for the demonstation

static void echo(SOCKET client_socket) {
    char echo_buffer[BUFFER_SIZE];
    int recv_size;
    time_t timestamp;

    if ((recv_size = recv(client_socket, echo_buffer, BUFFER_SIZE, 0)) < 0) {
        printf("\nError on receiving.\n");
        exit(1);
    }

    echo_buffer[recv_size] = '\0';
    time(&timestamp);
    fprintf("\nMessage from client: %s \t%s", echo_buffer, ctime(&timestamp));
}

int main(int argc, char* argv[]) {
    char buffer[BUFFER_SIZE];
    int port = DEFAULT_PORT; //remove me when using next line of code
    //char* port = atoi(argv[1]); //interpret port as int and not as string 
    SOCKADDR_STORAGE From; //stores the IPv6 address information
    int FromLen; //length of From
    SOCKET ServerSocket;
    struct sockaddr_in6 serv_addr;

    //argc[0] = sNummer, argc[1] = portnumber where server listens argc < 2
    if (argc < 1) {
        fprintf(stderr, "Application needs sNumber and portnumber as arguments.\n");
        exit(1);
    }

    printf("\nServer has been started.\n");

    /* Initialisiere TCP für Windows ("winsock"). */
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(1, 1);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        printf("\nError on initialising Winsock.\n");
        exit(1);
    }   
    else {
        printf("\nWinsock initialised.\n");
    }

    // Open a socket with the correct address family for this address.
    ServerSocket = socket(DEFAULT_FAMILY, DEFAULT_SOCKTYPE, 0);
    if (ServerSocket < 0) {
        printf("\nError opening socket.\n");
        exit(1);
    }
    else {
        printf("\nSocket has been opened.\n");
    }

    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = DEFAULT_FAMILY;
    serv_addr.sin6_addr = in6addr_loopback;
    serv_addr.sin6_port = htons(port);

    // bind() associates a local address and port combination with the socket just created
    if (bind(ServerSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nError on binding socket.\n");
        exit(1);
    }
    else {
        printf("\nBinding successful.\n");
    }

    //listen() for incoming connections
    if (listen(ServerSocket, 5) < 0) {
        printf("\nError on listening.\n");
        exit(1);
    }
    else {
        printf("\nListening on port %d.\n", port);
    }


    //accept() a new connection
    while (1) {
        SOCKET NewSocket;
        NewSocket = accept(ServerSocket, (LPSOCKADDR)&From, &FromLen);
        if (NewSocket < 0) {
            printf("\nError on accept.\n");
            exit(1);
        }

        echo(ServerSocket);

        //close() the socket
        closesocket(NewSocket);
    }
    return 0;
}