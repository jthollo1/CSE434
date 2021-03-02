#include "defns.h"

#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket(), connect(), sendto(), and recvfrom()
#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include <pthread.h>

// Declare global constants
const int USER_MAX = 50;
const int LIST_MAX = 50;
const int STRING_MAX = 50;
const int IP_MAX = 20;
const int RET_MAX = 10;

void *serverThread(void *vargp)
{
	while(1)
	{
	    sleep(1);
	    printf("Thread running... \n");
	}
}

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
    printf("|1. create:      Create new contact list           |\n");
    printf("|2. query-lists: Search for contact lists to join  |\n");
    printf("|3. join:        Join a contact list               |\n");
    printf("|4. leave        Leave a contact list              |\n");
    printf("|5. exit:        Exit Instant Messenger            |\n");
    printf("|6. im-start     Start an instant message          |\n");
    printf("|7. im-complete  Completed the instant message     |\n");
    printf("|8. save:        Save contact info                 |\n");
    printf("+--------------------------------------------------+\n");
}

// This function initializes/clears dataStruct
struct dataStruct initStruct(struct dataStruct data)
{
	data.command = -1;

	strcpy(data.listName, "");
	strcpy(data.contactName, "");
	memset(data.contactLists, 0, sizeof data.contactLists[0][0] * 50 * 50);

	strcpy(data.IP, "");
	data.port = 0;

	strcpy(data.fileName, "");

	strcpy(data.returnCode, "");

	return data;
}

// This function will send the data struct to the server
struct dataStruct sendStruct(int sock, struct sockaddr_in echoServAddr, struct dataStruct data)
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
		case 0:
			printf("Registering user: %s, IP: %s, port: %hu\n", data.contactName, data.IP, data.port);
			break;

		case 1:
			printf("Creating the %s contact list.\n", data.listName);
			break;

		case 2:
			printf("Querying for contact lists.\n");
			break;

		case 3:
			printf("%s is joining the %s contact list.\n", data.contactName, data.listName);
			break;

		case 4:
			printf("%s is leaving the %s contact list.\n", data.contactName, data.listName);
			break;

		case 5:
			printf("%s is exiting.\n", data.contactName);
			break;

		case 6:
			printf("%s is starting an IM on the %s contact list.\n", data.contactName, data.listName);
			break;

		case 7:
			printf("%s has completed an IM on the %s contact list.\n", data.contactName, data.listName);
			break;

		case 8:
			printf("Saving file named: %s\n", data.fileName);
			break;

		default:
			printf("Error: Bad command in dataStruct.\n");
			exit(1);
			break;
		}
	}

	// Receive a response
	fromSize = sizeof(fromAddr);

	if((nBytes = recvfrom(sock, &data, sizeof(struct dataStruct), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct dataStruct))
	{
		DieWithError("recvfrom() failed.\n");
	}

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: Received a packet from unknown source.\n");
		exit(1);
	}

	if(strcmp(data.returnCode, "") == 0)
	{
		strcpy(data.returnCode, "FAILURE");
	}

	printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg

	return data;
}

int main(int argc, char *argv[])
{
	int sock;                        // Socket descriptor
    struct sockaddr_in echoServAddr; // Echo server address
    //struct sockaddr_in echoClntAddr;     // Client address
    unsigned short echoServPort;     // Echo server port
    char *servIP;                    // IP address of server
    int selection;                   // User menu selection
    struct dataStruct data;          // Data structure
    char clientName[STRING_MAX];     // Name attached to client
    int exitClient;                  // Used to end process
    pthread_t server;

    if (argc < 3)    // Test for correct number of arguments
    {
        fprintf(stderr,"Usage: %s <Server IP address> <Echo Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           // First arg: server IP address (dotted quad)
    echoServPort = atoi(argv[2]);  // Second arg: Use given port, if any

	printf("Arguments passed: server IP %s, port %d\n", servIP, echoServPort);

    // Create a datagram/UDP socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DieWithError("socket() failed");
    }

    // Construct the server address structure
    memset(&echoServAddr, 0, sizeof(echoServAddr));    // Zero out structure
    echoServAddr.sin_family = AF_INET;                 // Internet addr family
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  // Server IP address
    echoServAddr.sin_port = htons(echoServPort);       // Server port

    // Initialize data structure
    data = initStruct(data);

    // Start server thread
    pthread_create(&server, NULL, serverThread, NULL);

    while(strcmp(data.returnCode, "") == 0)
    {
    	while(strcmp(data.contactName, "") == 0 || sizeof(data.contactName) > STRING_MAX)
		{
			printf("\nContact name: ");
			scanf("%s", data.contactName);

			if(sizeof(data.contactName) > STRING_MAX)
			{
				printf("Error: Contact name too long\n");
				scanf("%*[^\n]"); // clear scanf
			}
			else
			{
				while(strcmp(data.IP, "") == 0 || sizeof(data.IP) > IP_MAX)
				{
					printf("IP: ");
					scanf("%s", data.IP);

					if(sizeof(data.IP) > IP_MAX)
					{
						printf("Error: IP too long\n\n");
						scanf("%*[^\n]"); // clear scanf
					}
					else
					{
						while(data.port < 13000|| data.port > 13499)
						{
							printf("Port: ");
							scanf("%hu", &data.port);

							if(data.port < 13000 || data.port > 13499)
							{
								printf("Error: Port must be between 13000 and 13499.\n\n");
								scanf("%*[^\n]"); // clear scanf
							}
						}
					}
				}
			}
		}

		strcpy(clientName, data.contactName); // Save name to client
		data.command = 0; // Register command

		data = sendStruct(sock, echoServAddr, data);

		// Clear fields if register failed
		if(strcasecmp(data.returnCode, "SUCCESS") != 0)
		{
			data = initStruct(data);
		}
    }

    exitClient = 0; // initialize exit Client

    while(exitClient == 0)
    {
		menu();
		printf("\nSelection: ");
		scanf("%d", &selection);

		data = initStruct(data);

		switch(selection)
		{
		case 1: // create contact list
			printf("Selected: create\n\n");
			data.command = 1;

			printf("Contact list name: ");
			scanf("%s", data.listName);

			sendStruct(sock, echoServAddr, data);
			break;

		case 2: // query for contact lists
			printf("selected: query-lists\n\n");
			data.command = 2;

			data = sendStruct(sock, echoServAddr, data);

			if(strcmp(data.returnCode, "SUCCESS") != 0 && strcmp(data.returnCode, "FAILURE") != 0)
			{
				int listCount = atoi(data.returnCode);

				for(int i = 0; i < listCount; i++)
				{
					printf("%d: %s\n", i + 1, data.contactLists[i]);
				}
				printf("\n");
			}
			break;

		case 3: // join list
			printf("Selected: join\n\n");
			data.command = 3;

			printf("Contact list name: ");
			scanf("%s", data.listName);
			strcpy(data.contactName, clientName); // Using client bound name

			sendStruct(sock, echoServAddr, data);
			break;

		case 4: // leave list
			printf("Selected: leave\n\n");
			data.command = 4;

			printf("Contact list name: ");
			scanf("%s", data.listName);
			strcpy(data.contactName, clientName); // Using client bound name

			sendStruct(sock, echoServAddr, data);
			break;

		case 5: // exit messaging
			printf("Selected: exit\n\n");
			data.command = 5;

			strcpy(data.contactName, clientName); // Using client bound name

			data = sendStruct(sock, echoServAddr, data);

			if(strcmp(data.returnCode, "SUCCESS") == 0)
			{
				exitClient = 1;
			}

			break;

		case 6: // start IM
			printf("Selected: im-start\n\n");
			data.command = 6;

			printf("Contact list name: ");
			scanf("%s", data.listName);
			strcpy(data.contactName, clientName); // Using client bound name

			data = sendStruct(sock, echoServAddr, data);

			if(strcmp(data.returnCode, "SUCCESS") != 0 && strcmp(data.returnCode, "FAILURE") != 0)
			{
				int listCount = atoi(data.returnCode);

				for(int i = 0; i < listCount; i++)
				{
					printf("%d: %s   %s   %d\n", i + 1, data.userList[i].contactName, data.userList[i].IP, data.userList[i].port);
				}
				printf("\n");
			}
			break;

		case 7: // complete IM
			data.command = 7;

			printf("Contact list name: ");
			scanf("%s", data.listName);
			strcpy(data.contactName, clientName); // Using client bound name

			sendStruct(sock, echoServAddr, data);
			break;

		case 8: // save contacts
			printf("Selected: save\n\n");
			data.command = 8;

			printf("File name: ");
			scanf("%s", data.fileName);

			sendStruct(sock, echoServAddr, data);
			break;

		default:
			printf("Error: That was not a valid selection.\n\n");
			scanf("%*[^\n]"); // clear scanf
			break;
		}
    }
    
    close(sock);
    exit(0);
}