

#include "defns.h"

#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket(), connect(), sendto(), and recvfrom()
#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()

#define ITERATIONS	5

void DieWithError(const char *errorMessage) /* External error handling function */
{
    perror(errorMessage);
    exit(1);
}

void menu()
{
    // Display Menu
    printf("+--------------------------------------------------+\n");
    printf("|            Welcome to Instant Messenger          |\n");
    printf("|      CSE 434 / Group 24 / jthollo1 / anispas     |\n");
    printf("|--------------------------------------------------|\n");
    printf("|  Please make a selection by typing in its number |\n");
    printf("|                                                  |\n");
    printf("|1. register    <contact-name> <IP-address> <port> |\n");
    printf("|2. create      <contact-list-name>                |\n");
    printf("|3. query-lists                                    |\n");
    printf("|4. join        <contact-list-name> <contact-name> |\n");
  //printf("|5. leave       <contact-list-name> <contact-name> |\n");
    printf("|5. exit        <contact-name>                     |\n");
  //printf("|7. im-start    <contact-list-name> <contact-name> |\n");
  //printf("|8. im-complete <contact-list-name> <contact-name> |\n");
    printf("|6. save        <file-name>                        |\n");
    printf("+--------------------------------------------------+\n");
}

void reg(int sock, struct sockaddr_in echoServAddr, struct regUser user)
{
    struct sockaddr_in fromAddr;     // Source address of echo
    unsigned int fromSize;           // In-out of address size for recvfrom()
    int nBytes;              		 // Length of received response

	// Send the struct to the server
	if(sendto(sock, &user, sizeof(struct regUser), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct regUser))
	{
   		DieWithError("sendto() sent a different number of bytes than expected");
	}
	else
	{
		printf( "\nRegistering user <%s,%s,%d>\n", user.contactName, user.IP, user.port);
	}

	// Receive a response
	fromSize = sizeof(fromAddr);

	if((nBytes = recvfrom(sock, &user, sizeof(struct regUser), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct regUser))
	{
		DieWithError("recvfrom() failed");
	}

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}

	printf("\nServer response: %s\n", user.returnCode); // Print the echoed arg
}

void create(int sock, struct sockaddr_in echoServAddr, struct createList create)
{
    struct sockaddr_in fromAddr;     // Source address of echo
    unsigned int fromSize;           // In-out of address size for recvfrom()
    int nBytes;              		 // Length of received response

	// Send the struct to the server
	if(sendto(sock, &create, sizeof(struct createList), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct createList))
	{
   		DieWithError("sendto() sent a different number of bytes than expected");
	}
	else
	{
		printf( "\nCreating contact list <%s>\n", create.listName);
	}

	// Receive a response
	fromSize = sizeof(fromAddr);

	if((nBytes = recvfrom(sock, &create, sizeof(struct createList), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct createList))
	{
		DieWithError("recvfrom() failed");
	}

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}

	printf("\nServer response: %s\n", create.returnCode); // Print the echoed arg
}

void query(int sock, struct sockaddr_in echoServAddr, struct queryList query)
{
    struct sockaddr_in fromAddr;     // Source address of echo
    unsigned int fromSize;           // In-out of address size for recvfrom()
    int nBytes;              		 // Length of received response

	// Send the struct to the server
	if(sendto(sock, &query, sizeof(struct queryList), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct queryList))
	{
   		DieWithError("sendto() sent a different number of bytes than expected");
	}
	else
	{
		printf( "\nQuerying for contact lists\n");
	}

	// Receive a response
	fromSize = sizeof(fromAddr);

	if((nBytes = recvfrom(sock, &query, sizeof(struct queryList), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct queryList))
	{
		DieWithError("recvfrom() failed");
	}

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}

	printf("\nServer response: %s\n", query.returnCode); // Print the echoed arg
}

void join(int sock, struct sockaddr_in echoServAddr, struct joinList joinL){
    struct sockaddr_in fromAddr;     // Source address of echo
    unsigned int fromSize;           // In-out of address size for recvfrom()
    int nBytes;              		 // Length of received response

	// Receive a response
	fromSize = sizeof(fromAddr);

    //Checks to see if contact exists already
    if(sendto(sock)){

    	printf(stderr,"Error: Contact is already in ongoing instant message.\n");
    	exit(1);
    }

    else{

    }

    printf("\nServer response: %s\n", joinL.returnCode); // Print the echoed arg

}

//void leave(int sock, struct sockaddr_in echoServAddr){}

void exitProg(int sock, struct sockaddr_in echoServAddr, struct exitP e)
{
    struct sockaddr_in fromAddr;     // Source address of echo
    unsigned int fromSize;           // In-out of address size for recvfrom()
    int nBytes;              		 // Length of received response

	// Send the struct to the server
	if(sendto(sock, &e, sizeof(struct exitP), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct exitP))
	{
   		DieWithError("sendto() sent a different number of bytes than expected");
	}
	else
	{
		printf( "\n<%s> is exiting\n", e.contactName);
	}

	// Receive a response
	fromSize = sizeof(fromAddr);

	if((nBytes = recvfrom(sock, &e, sizeof(struct exitP), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct exitP))
	{
		DieWithError("recvfrom() failed");
	}

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}

	printf("\nServer response: %s\n", e.returnCode); // Print the echoed arg
}

void save(int sock, struct sockaddr_in echoServAddr, struct saveFile s)
{
    struct sockaddr_in fromAddr;     // Source address of echo
    unsigned int fromSize;           // In-out of address size for recvfrom()
    int nBytes;              		 // Length of received response

	// Send the struct to the server
	if(sendto(sock, &create, sizeof(struct saveFile), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct saveFile))
	{
   		DieWithError("sendto() sent a different number of bytes than expected");
	}
	else
	{
		printf( "\nSaving file <%s>\n", s.fileName);
	}

	// Receive a response
	fromSize = sizeof(fromAddr);

	if((nBytes = recvfrom(sock, &create, sizeof(struct saveFile), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct saveFile))
	{
		DieWithError("recvfrom() failed");
	}

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}

	printf("\nServer response: %s\n", s.returnCode); // Print the echoed arg
}

int main(int argc, char *argv[])
{
	int sock;                        // Socket descriptor
    struct sockaddr_in echoServAddr; // Echo server address
    unsigned short echoServPort;     // Echo server port
    char *servIP;                    // IP address of server

    int selection;

    struct regUser user;


    if (argc < 3)    // Test for correct number of arguments
    {
        fprintf(stderr,"Usage: %s <Server IP address> <Echo Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           // First arg: server IP address (dotted quad)
    echoServPort = atoi(argv[2]);  // Second arg: Use given port, if any

	printf( "Arguments passed: server IP %s, port %d\n", servIP, echoServPort );

    // Create a datagram/UDP socket

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    // Construct the server address structure

    memset(&echoServAddr, 0, sizeof(echoServAddr));    // Zero out structure
    echoServAddr.sin_family = AF_INET;                 // Internet addr family
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  // Server IP address
    echoServAddr.sin_port   = htons(echoServPort);     // Server port


    while(1)
    {
		menu();
		printf("\nSelection: ");
		scanf("%d", &selection);

		switch(selection)
		{
		case 1:
			printf("Selected register\n\n");

			printf("Contact name: ");
			scanf("%s", user.contactName);
			printf("IP address: ");
			scanf("%s", user.IP);
			printf("Port: ");
			scanf("%hu", &user.port);
			printf("\n");

			reg(sock, echoServAddr, user);
			break;

		case 2:
			printf("Selected: create\n\n");
			break;

		case 3:
			printf("selected: query-lists\n\n");
			break;

		case 4:
			printf("Selected: join\n\n");
			break;

		case 5:
			printf("Selected: exit\n\n");
			break;

		case 6:
			printf("Selected: save\n\n");
			break;

		default:
			printf("That was not a valid selection.\n\n");
			scanf("%*[^\n]");
			break;
		}
    }
    
    close(sock);
    exit(0);
}

