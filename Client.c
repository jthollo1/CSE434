

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

struct dataStruct initStruct(struct dataStruct data)
{
	data.command = 0;

	strcpy(data.listName, "");
	strcpy(data.contactName, "");
	memset(data.contactList, 0, sizeof data.contactList[0][0] * 50 * 50);

	strcpy(data.IP, "");
	data.port = 0;

	strcpy(data.fileName, "");

	strcpy(data.returnCode, "");

	return data;
}

void sendStruct(int sock, struct sockaddr_in echoServAddr, struct dataStruct data)
{
    struct sockaddr_in fromAddr;     // Source address of echo
    unsigned int fromSize;           // In-out of address size for recvfrom()
    int nBytes;              		 // Length of received response

	// Send the struct to the server
	if(sendto(sock, &data, sizeof(struct dataStruct), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct dataStruct))
	{
   		DieWithError("sendto() sent a different number of bytes than expected");
	}
	else
	{
		switch(data.command)
		{
		case 1:
			printf("Registering user: %s IP: %s port: %hu\n", data.contactName, data.IP, data.port);
			break;

		case 2:
			printf("Creating contact list named: %s\n", data.listName);
			break;

		case 3:
			printf("Querying for contact lists\n");
			break;

		case 4:
			printf("User %s is joining contact list named: %s\n", data.contactName, data.listName);
			break;

		case 5:
			printf("%s is exiting\n", data.contactName);
			break;

		case 6:
			printf("Saving file named: %s\n", data.fileName);
			break;

		default:
			printf("Error: bad command in dataStruct");
			exit(1);
			break;
		}
	}

	// Receive a response
	fromSize = sizeof(fromAddr);

	if((nBytes = recvfrom(sock, &data, sizeof(struct dataStruct), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct dataStruct))
	{
		DieWithError("recvfrom() failed");
	}

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: received a packet from unknown source.\n");
		exit(1);
	}

	if(strcmp(data.returnCode, "SUCCESS") != 0)
	{
		printf("Server response: FAILURE\n\n");
	}
	else
	{
		printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg
	}
}

int main(int argc, char *argv[])
{
	int sock;                        // Socket descriptor
    struct sockaddr_in echoServAddr; // Echo server address
    unsigned short echoServPort;     // Echo server port
    char *servIP;                    // IP address of server
    int selection;                   // User menu selection
    struct dataStruct data;          // Data structure

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

		initStruct(data);

		switch(selection)
		{
		case 1: // register
			printf("Selected register\n\n");
			data.command = 1;

			printf("Contact name: ");
			scanf("%s", data.contactName);
			printf("IP address: ");
			scanf("%s", data.IP);
			printf("Port: ");
			scanf("%hu", &data.port);
			printf("\n");

			sendStruct(sock, echoServAddr, data);
			break;

		case 2: // create contact list
			printf("Selected: create\n\n");
			data.command = 2;

			printf("Contact list name: ");
			scanf("%s", data.listName);

			sendStruct(sock, echoServAddr, data);
			break;

		case 3: // query for contact lists
			printf("selected: query-lists\n\n");
			data.command = 3;

			sendStruct(sock, echoServAddr, data);
			break;

		case 4: // join list
			printf("Selected: join\n\n");
			data.command = 4;

			printf("Contact list name: ");
			scanf("%s", data.listName);
			printf("Contact name: ");
			scanf("%s", data.contactName);

			sendStruct(sock, echoServAddr, data);
			break;

		case 5: // exit messaging
			printf("Selected: exit\n\n");
			data.command = 5;

			printf("Contact name: ");
			scanf("%s", data.contactName);

			sendStruct(sock, echoServAddr, data);
			break;

		case 6: // save contacts
			printf("Selected: save\n\n");
			data.command = 6;

			printf("File name: ");
			scanf("%s", data.fileName);

			sendStruct(sock, echoServAddr, data);
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

