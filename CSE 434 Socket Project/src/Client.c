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
const int MSG_MAX = 200;

// Declare global variables
char lastPrint[50]; // last printed string

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
    printf("|7. save:        Save contact info                 |\n");
    printf("+--------------------------------------------------+\n\n");
}

// This function initializes/clears dataStruct
struct dataStruct initStruct(struct dataStruct data)
{
	data.command = -1;

	strcpy(data.listName, "");
	strcpy(data.contactName, "");
	memset(data.contactLists, 0, sizeof data.contactLists[0][0] * 50 * 50);
	memset(data.userList, 0, sizeof data.userList[0] * 50);

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
			printf("Registering user: %s, port: %hu\n", data.contactName, data.port);
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
			printf("Saving file named: %s\n", data.fileName);
			break;

		case 8:
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

	return data;
}

// This function initializes/clears a message
struct msgStruct initMsg(struct msgStruct msg)
{
	strcpy(msg.message, "");
	memset(msg.userList, 0, sizeof msg.userList[0] * 50);

	strcpy(msg.returnCode, "");

	return msg;
}

// This function will send the message to a client
struct msgStruct sendMsg(int sock, struct sockaddr_in echoServAddr, struct msgStruct msg)
{
    struct sockaddr_in fromAddr;     // Source address of echo
    unsigned int fromSize;           // In-out of address size for recvfrom()
    int nBytes;              		 // Length of received response

	// Send the struct to the server
	if(sendto(sock, &msg, sizeof(struct msgStruct), 0, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != sizeof(struct msgStruct))
	{
   		DieWithError("sendto() sent a different number of bytes than expected");
	}

	// Receive a response
	fromSize = sizeof(fromAddr);

	if((nBytes = recvfrom(sock, &msg, sizeof(struct msgStruct), 0, (struct sockaddr *) &fromAddr, &fromSize)) > sizeof(struct msgStruct))
	{
		DieWithError("recvfrom() failed.\n");
	}

	if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
	{
		fprintf(stderr,"Error: Received a packet from unknown source.\n");
		exit(1);
	}

	if(strcmp(msg.returnCode, "") == 0)
	{
		strcpy(msg.returnCode, "FAILURE");
	}

	return msg;
}

// This function is run as a seperate thread that acts as a server for each client process
void *serverThread(void* arguments)
{
	struct serverStruct *args = arguments;

	// Declare variables
    int sock;                            // Socket
    int nextSock;                        // Next client's socket
    struct sockaddr_in echoServAddr;     // Local address
    struct sockaddr_in echoClntAddr;     // Client address
    struct sockaddr_in echoNextAddr;     // Next client address
    struct user nextUser;                // Next user structure
    unsigned int cliAddrLen;             // Length of incoming message
    unsigned short echoServPort;         // Server port
    unsigned short echoNextPort;         // Next client port
    char *nextIP;                        // IP address of next client
    int recvMsgSize;                     // Size of received message
    struct msgStruct msg;                // Message structure

    echoServPort = args->port;           // assigned port

    // Create socket for sending/receiving datagrams
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DieWithError("socket() failed");
    }

    // Construct local address structure
    memset(&echoServAddr, 0, sizeof(echoServAddr));   // Zero out structure
    echoServAddr.sin_family = AF_INET;                // Internet address family
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    echoServAddr.sin_port = htons(echoServPort);      // Local port

    // Bind to the local address
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
    {
        DieWithError("bind() failed");
    }

	for (;;) // Run forever
	{
        // Set the size of the in-out parameter
        cliAddrLen = sizeof(echoClntAddr);

        // Block until receive command from a client
        if ((recvMsgSize = recvfrom(sock, &msg, sizeof(struct msgStruct), 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
        {
            DieWithError("recvfrom() failed");
        }
        else
        {
        	// Only print message if it's not at the sender
        	if(msg.size > 0)
        	{
        		printf("\33[2K"); // Clear current line
        		printf("\r");     // Return cursor
            	printf("New message from %s:\n", msg.userList[msg.size-1].contactName);
            	printf("%s\n", msg.message);
            	printf("%s", lastPrint);
            	fflush(stdout);   // Flush output for newline

    			// Rearranging next contact list
    			// Save first user on list
    			strcpy(nextUser.contactName, msg.userList[0].contactName);
    			strcpy(nextUser.IP, msg.userList[0].IP);
    			nextUser.port = msg.userList[0].port;

    			// Delete first user (next user)
    			strcpy(msg.userList[0].contactName, "");
    			strcpy(msg.userList[0].IP, "");
    			msg.userList[0].port = 0;

            	// Decrement users remaining
    			msg.size--;

    			// Building next contact list
    			for(int i = 0; i < msg.size; i++)
    			{
    				// Move users up 1
    				// i+1 because rearranging deleted first user
    				strcpy(msg.userList[i].contactName, msg.userList[i+1].contactName);
    				strcpy(msg.userList[i].IP, msg.userList[i+1].IP);
    				msg.userList[i].port = msg.userList[i+1].port;

					// Delete original user
    				strcpy(msg.userList[i+1].contactName, "");
    				strcpy(msg.userList[i+1].IP, "");
    				msg.userList[i+1].port = 0;
    			}

    			nextIP = nextUser.IP;         // Client IP address (dotted quad)
    			echoNextPort = nextUser.port; // Use given port

    			// Create a datagram/UDP socket
    			if ((nextSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    			{
    				DieWithError("socket() failed");
    			}

    			// Construct the server address structure
    			memset(&echoNextAddr, 0, sizeof(echoNextAddr));    // Zero out structure
    			echoNextAddr.sin_family = AF_INET;                 // Internet addr family
    			echoNextAddr.sin_addr.s_addr = inet_addr(nextIP);  // Client IP address
    			echoNextAddr.sin_port = htons(echoNextPort);       // Client port

    			msg = sendMsg(nextSock, echoNextAddr, msg);

    			close(nextSock);
        	}
        	else // This is the sender
        	{
        		// Message has propagated to all users
        		strcpy(msg.returnCode, "SUCCESS");
        	}

            // Send received datagram back to the client
            if (sendto(sock, &msg, sizeof(struct msgStruct), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(struct msgStruct))
            {
                DieWithError("sendto() sent a different number of bytes than expected");
            }
        }
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int sock;                        // Socket descriptor
	int clientSock;                  // Client socket descriptor
    struct sockaddr_in echoServAddr; // Server address
    unsigned short echoServPort;     // Server port
    struct sockaddr_in echoClntAddr; // Client address
    unsigned short echoClntPort;     // Client port
    char *servIP;                    // IP address of server
    char *clntIP;                    // IP address of client
    int selection;                   // User menu selection
    struct dataStruct data;          // Data structure
    struct serverStruct args;        // Server structure
    struct msgStruct msg;            // Message structure
    struct user nextUser;            // foundUser structure
    char clientName[STRING_MAX];     // Name attached to client
    char listName[STRING_MAX];       // Name attached to contact lists
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

		strcpy(clientName, data.contactName); // Save name to client
		data.command = 0; // Register command

		data = sendStruct(sock, echoServAddr, data);

		// Clear fields if register failed
		if(strcasecmp(data.returnCode, "SUCCESS") != 0)
		{
			data = initStruct(data);
		}
		else
		{
			args.port = data.port;

		    // Start server thread
		    pthread_create(&server, NULL, serverThread, &args);
		}
    }

    exitClient = 0; // initialize exit Client

    while(exitClient == 0)
    {
		menu();
		printf("Selection: ");
		strcpy(lastPrint, "Selection: ");
		scanf("%d", &selection);

		data = initStruct(data); // clear data

		switch(selection)
		{
		case 1: // create contact list
			printf("Selected: create\n\n");
			data.command = 1;

			printf("Contact list name: ");
			strcpy(lastPrint, "Contact list name: ");
			scanf("%s", data.listName);

			data = sendStruct(sock, echoServAddr, data); // send to server and save
			printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg

			break;

		case 2: // query for contact lists
			printf("selected: query-lists\n\n");
			data.command = 2;

			data = sendStruct(sock, echoServAddr, data); // send to server and save
			printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg

			// Check if the return code is a number
			if(strcmp(data.returnCode, "FAILURE") != 0)
			{
				int listCount = atoi(data.returnCode);

				// Print contact lists
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
			strcpy(lastPrint, "Contact list name: ");
			scanf("%s", data.listName);
			strcpy(data.contactName, clientName); // Using client bound name

			data = sendStruct(sock, echoServAddr, data); // send to server and save
			printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg

			break;

		case 4: // leave list
			printf("Selected: leave\n\n");
			data.command = 4;

			printf("Contact list name: ");
			strcpy(lastPrint, "Contact list name: ");
			scanf("%s", data.listName);
			strcpy(data.contactName, clientName); // Using client bound name

			data = sendStruct(sock, echoServAddr, data); // send to server and save
			printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg

			break;

		case 5: // exit messaging
			printf("Selected: exit\n\n");
			data.command = 5;

			strcpy(data.contactName, clientName); // Using client bound name

			data = sendStruct(sock, echoServAddr, data); // send to server and save
			printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg

			if(strcmp(data.returnCode, "SUCCESS") == 0)
			{
				exitClient = 1;
			}

			break;

		case 6: // start IM
			printf("Selected: im-start\n\n");
			data.command = 6;

			printf("Contact list name: ");
			strcpy(lastPrint, "Contact list name: ");
			scanf("%s", listName);
			strcpy(data.listName, listName);
			strcpy(data.contactName, clientName); // Using client bound name

			data = sendStruct(sock, echoServAddr, data);

			// Check if IM was successfully started
			if(strcmp(data.returnCode, "FAILURE") != 0)
			{
				// Request succeeded
				printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg

				// Convert string to integer
				int listCount = atoi(data.returnCode);

				// Display list of users
				for(int i = 0; i < listCount; i++)
				{
					printf("%d: %s   %s   %d\n", i + 1, data.userList[i].contactName, data.userList[i].IP, data.userList[i].port);
				}
				printf("\n"); // Spacer

				// Rearranging next contact list
				// Copy first user to back of list
				strcpy(data.userList[listCount].contactName, data.userList[0].contactName);
				strcpy(data.userList[listCount].IP, data.userList[0].IP);
				data.userList[listCount].port = data.userList[0].port;

				// Save second user on list
				strcpy(nextUser.contactName, data.userList[1].contactName);
				strcpy(nextUser.IP, data.userList[1].IP);
				nextUser.port = data.userList[1].port;

				// Initialize/clear message
				msg = initMsg(msg);
				msg.size = listCount - 1;

				// Building next contact list
				for(int i = 0; i < msg.size; i++)
				{
					// i+2 because don't need first two users
					strcpy(msg.userList[i].contactName, data.userList[i+2].contactName);
					strcpy(msg.userList[i].IP, data.userList[i+2].IP);
					msg.userList[i].port = data.userList[i+2].port;
				}

			    clntIP = nextUser.IP;         // Client IP address (dotted quad)
			    echoClntPort = nextUser.port; // Use given port

			    // Create a datagram/UDP socket
			    if ((clientSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
			    {
			        DieWithError("socket() failed");
			    }

			    // Construct the server address structure
			    memset(&echoClntAddr, 0, sizeof(echoClntAddr));    // Zero out structure
			    echoClntAddr.sin_family = AF_INET;                 // Internet addr family
			    echoClntAddr.sin_addr.s_addr = inet_addr(clntIP);  // Client IP address
			    echoClntAddr.sin_port = htons(echoClntPort);       // Client port

				printf("Message: ");
				strcpy(lastPrint, "Message: ");
				fgetc(stdin); // capture newline
				fgets(msg.message, MSG_MAX, stdin); // Read message
				printf("Sending message... \n");

			    // Send message
			    msg = sendMsg(clientSock, echoClntAddr, msg);
			    printf("Server response: %s\n\n", msg.returnCode); // Print the echoed arg

			    // Regardless of success or not, server socket closes and client sends im-complete
				close(clientSock);

				// Unlock contact list
			    data = initStruct(data);              // Clear data
			    data.command = 8;                     // im-complete command

			    strcpy(data.listName, listName);      // Using client bound name
			    strcpy(data.contactName, clientName); // Using client bound name
			    sendStruct(sock, echoServAddr, data); // send to server
			}
			else
			{
				// Request failed
				printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg
			}

			break;

		case 7: // save contacts
			printf("Selected: save\n\n");
			data.command = 7;

			printf("File name: ");
			strcpy(lastPrint, "File name: ");
			scanf("%s", data.fileName);

			data = sendStruct(sock, echoServAddr, data); // send to server
			printf("Server response: %s\n\n", data.returnCode); // Print the echoed arg

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

