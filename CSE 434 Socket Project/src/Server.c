

#include "defns.h"

#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket() and bind()
#include <arpa/inet.h>  // for sockaddr_in and inet_ntoa()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()

void DieWithError(const char *errorMessage) // External error handling function
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                        // Socket
    struct sockaddr_in echoServAddr; // Local address
    struct sockaddr_in echoClntAddr; // Client address
    unsigned int cliAddrLen;         // Length of incoming message
    unsigned short echoServPort;     // Server port
    int recvMsgSize;                 // Size of received message
    struct dataStruct data;          // Data structure

    if (argc != 2)         // Test for correct number of parameters
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  // First arg:  local port

    // Create socket for sending/receiving datagrams
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    // Construct local address structure
    memset(&echoServAddr, 0, sizeof(echoServAddr));   // Zero out structure
    echoServAddr.sin_family = AF_INET;                // Internet address family
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    echoServAddr.sin_port = htons(echoServPort);      // Local port

    // Bind to the local address
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
  
    for (;;) // Run forever
    {
        // Set the size of the in-out parameter
        cliAddrLen = sizeof(echoClntAddr);

        printf("Waiting for client command...\n");

        // Block until receive command from a client
        if ((recvMsgSize = recvfrom(sock, &data, sizeof(struct dataStruct), 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
        {
            DieWithError("recvfrom() failed");
        }
        else
        {
        	printf("\nServer got request from client at IP address: %s\n", inet_ntoa(echoClntAddr.sin_addr));

    		switch(data.command)
    		{
    		case 1: // register
                printf("Server received register command: contactName: %s, IP: %s, port: %hu\n\n", data.contactName, data.IP, data.port);

        		// Update return code
        		strcpy(data.returnCode, "SUCCESS");
    			break;

    		case 2: // create contact list
                printf("Server received create command: listName: %s\n\n", data.listName);

        		// Update return code
        		strcpy(data.returnCode, "SUCCESS");
    			break;

    		case 3: // query for contact lists
    			printf("Server received query command\n\n");

        		// Update return code
        		strcpy(data.returnCode, "SUCCESS");
    			break;

    		case 4: // join list
    			printf("Server received join command: listName: %s, contactName: %s\n\n", data.listName, data.contactName);

        		// Update return code
        		strcpy(data.returnCode, "SUCCESS");
    			break;

    		case 5: // exit messaging
    			printf("Server received exit command: contactName: %s\n\n", data.contactName);

        		// Update return code
        		strcpy(data.returnCode, "SUCCESS");
    			break;

    		case 6: // save contacts
    			printf("Server received save command: fileName: %s\n\n", data.fileName);

        		// Update return code
        		strcpy(data.returnCode, "SUCCESS");
    			break;

    		default:
    			printf("Error: invalid command recieved.\n\n");
    			scanf("%*[^\n]");
    			break;
    		}

            // Send received datagram back to the client
            if (sendto(sock, &data, sizeof(struct dataStruct), 0, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != sizeof(struct dataStruct))
            {
                DieWithError("sendto() sent a different number of bytes than expected");
            }
        }
    }
    // NOT REACHED
}

