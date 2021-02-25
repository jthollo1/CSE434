#include "defns.h"

#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket() and bind()
#include <arpa/inet.h>  // for sockaddr_in and inet_ntoa()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()

// Declare global constants
const int USER_MAX = 50;
const int LIST_MAX = 50;
const int STRING_MAX = 50;

void DieWithError(const char *errorMessage) // External error handling function
{
    perror(errorMessage);
    exit(1);
}

// Function returns a found user
struct foundUser searchUser(struct user userList[], int userListSize, char name[])
{
	struct foundUser found;
	found.found = 0;

	for(int i = 0; i < userListSize; i++)
	{
		if(strcasecmp(name, userList[i].contactName) == 0)
		{
			strcpy(found.contactName, userList[i].contactName);
			strcpy(found.IP, userList[i].IP);
			found.port = userList[i].port;
			found.index = i;
			found.found = 1;
			break;
		}
	}

	return found;
}

// Function returns the found contact list index
struct foundContact searchList(struct cList contactList[], int contactListSize, char name[])
{
	struct foundContact found;
	found.found = 0;

	for(int i = 0; i < contactListSize; i++)
	{
		if(strcasecmp(name, contactList[i].listName) == 0)
		{
			found.index = i;
			found.found = 1;
			break;
		}
	}

	return found;
}

int main(int argc, char *argv[])
{
	// Declare variables
    int sock;                        // Socket
    struct sockaddr_in echoServAddr; // Local address
    struct sockaddr_in echoClntAddr; // Client address
    unsigned int cliAddrLen;         // Length of incoming message
    unsigned short echoServPort;     // Server port
    int recvMsgSize;                 // Size of received message
    struct dataStruct data;          // Data structure
	struct foundUser user;           // foundUser structure
	struct foundContact contact;     // foundContact structure
	int size;                        // userList size in contact list

    struct user userList[USER_MAX];  // Array of users
    int userNum = 0;                 // Number of users in user list

    struct cList contactList[LIST_MAX];  // Array of contact lists
    int listNum = 0;                     // Number of contact lists in array

    char strLen[STRING_MAX];             // String for return code
    char fileName[STRING_MAX];           // String for file output

    if (argc != 2)         // Test for correct number of parameters
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  // First arg:  local port

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

        printf("Waiting for client command...\n");

        // Block until receive command from a client
        if ((recvMsgSize = recvfrom(sock, &data, sizeof(struct dataStruct), 0, (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
        {
            DieWithError("recvfrom() failed");
        }
        else
        {
        	printf("\nServer got request from client at IP address: %s\n", inet_ntoa(echoClntAddr.sin_addr));

    		switch(data.command) // switching on command sent from client
    		{
    		case 0: // register user
                printf("Server received register command. contactName: %s, IP: %s, port: %hu\n\n", data.contactName, data.IP, data.port);

                // Checking for max users
                if(userNum < USER_MAX)
                {
                	if(userNum == 0 || searchUser(userList, userNum, data.contactName).found == 0)
                	{
                		// assign new user using current userNum
                    	strcpy(userList[userNum].contactName, data.contactName);
                    	strcpy(userList[userNum].IP, data.IP);
                        userList[userNum].port = data.port;

                        userNum++; // added new user

                        printf("User count updated to: %d\n\n", userNum);

                		// Update return code
                		strcpy(data.returnCode, "SUCCESS");
                	}
                	else
                	{
                		printf("Error: Contact name already registered.\n\n");

                		// Update return code
                		strcpy(data.returnCode, "FAILURE");
                	}
                }
                else
                {
                	printf("Error: Maximum users registered.\n\n");

            		// Update return code
            		strcpy(data.returnCode, "FAILURE");
                }

    			break;

    		case 1: // create contact list
                printf("Server received create command. listName: %s\n\n", data.listName);

                // Checking for max lists
                if(listNum < LIST_MAX)
                {
                	if(listNum == 0 || searchList(contactList, listNum, data.listName).found == 0)
                	{
                		// assign new contact list using current listNum
                    	strcpy(contactList[listNum].listName, data.listName);

                    	listNum++; // added new contact list

                    	printf("Contact list count updated to: %d\n\n", listNum);

                		// Update return code
                		strcpy(data.returnCode, "SUCCESS");
                	}
                	else
                	{
                		printf("Error: Contact name already in contact list.\n\n");

                		// Update return code
                		strcpy(data.returnCode, "FAILURE");
                	}
                }
                else
                {
                	printf("Error: Maximum contact lists registered.\n\n");

            		// Update return code
            		strcpy(data.returnCode, "FAILURE");
                }

    			break;

    		case 2: // query for contact lists
    			printf("Server received query command.\n\n");

    			// Checking if there's a contact list
    			if(listNum > 0)
    			{
    				printf("Found %d contact lists.\n\n", listNum);

    				// Gathering contact lists
    				for(int i = 0; i < listNum; i++)
    				{
    					strcpy(data.contactList[i], contactList[i].listName);
    				}

    				// Converting listNum to string for return code
    				sprintf(strLen, "%d", listNum);

            		// Update return code
            		strcpy(data.returnCode, strLen);
    			}
    			else
    			{
    				printf("Error: No contact lists registered.\n\n");

            		// Update return code
            		strcpy(data.returnCode, "FAILURE");
    			}

    			break;

    		case 3: // join list
    			printf("Server received join command. listName: %s, contactName: %s\n\n", data.listName, data.contactName);

    			// Checking if there's both a user and contact list
    			if(listNum > 0 && userNum > 0)
    			{
    				user = searchUser(userList, userNum, data.contactName);
    				contact = searchList(contactList, listNum, data.listName);

    				// Checking if user name and contact name exist
    				if(user.found == 1 && contact.found == 1)
    				{
    					size = contactList[contact.index].size;

    					// List is empty
    					if(size == 0)
    					{
    						strcpy(contactList[contact.index].userList[size].contactName, user.contactName);
							strcpy(contactList[contact.index].userList[size].IP, user.IP);
							contactList[contact.index].userList[size].port = user.port;
							contactList[contact.index].size++;

							printf("User %s was added to the %s contact list at index %d.\n\n", contactList[contact.index].userList[size].contactName, contactList[contact.index].listName, size);

							// Update return code
							strcpy(data.returnCode, "SUCCESS");
    					}
    					else
    					{
    						user = searchUser(contactList[contact.index].userList, contactList[contact.index].size, data.contactName);

							// Checking if user already exists in the contact list
							if(user.found == 0)
							{
								//If user was not in the list, its data must be collected from the register
								user = searchUser(userList, userNum, data.contactName);

								strcpy(contactList[contact.index].userList[size].contactName, user.contactName);
								strcpy(contactList[contact.index].userList[size].IP, user.IP);
								contactList[contact.index].userList[size].port = user.port;
								contactList[contact.index].size++;

								printf("User %s was added to the %s contact list at index %d.\n\n", contactList[contact.index].userList[size].contactName, contactList[contact.index].listName, size);

								// Update return code
								strcpy(data.returnCode, "SUCCESS");
							}
							else
							{
								printf("Error: Contact name already in contact list.\n\n");

								// Update return code
								strcpy(data.returnCode, "FAILURE");
							}
    					}
    				}
    				else
    				{
    					printf("Error: Contact name or contact list not found.\n\n");

                		// Update return code
                		strcpy(data.returnCode, "FAILURE");
    				}
    			}
    			else
    			{
    				printf("Error: No users or contact lists registered.\n\n");

            		// Update return code
            		strcpy(data.returnCode, "FAILURE");
    			}

    			break;
    			
    		case 4://leave list
    			printf("Server recieved leave command. listName: %s contactName: %s\n\n", data.listName, data.contactName);
    			//checking to see if there's a user and a list
    			if(listNum > 0 && userNum > 0)
    			{
    				user = searchUser(userList, userNum, data.contactName);
    				contact = searchList(contactList, listNum, data.listName);
    				
    				// Checking if user name and contact name exist
    				if(user.found == 1 && contact.found == 1)
    				{
    				    size = contactList[contact.index].size;
    				    //checking to see if list is empty
    				    if(size == 0)
    				    {
    				    	printf("Error: No users in the contact lists registered.\n\n");
    				    	// Update return code
    				    	strcpy(data.returnCode, "FAILURE");
    				    }
    				    else
    				    {
    				    	user = searchUser(contactList[contact.index].userList, contactList[contact.index].size, data.contactName);
    				    	if(user.found == 1)
    				    	{
    				    		
    				    		// Remove user from list
    				    		strcpy(contactList[contact.index].userList[user.index].contactName, "");
    				    		strcpy(contactList[contact.index].userList[user.index].IP, "");
    				    		contactList[contact.index].userList[user.index].port = 0;
    				    		printf("%s was removed from the %s contact list.\n", data.contactName, contactList[contact.index].listName);
    				    		// Rebuild list
    				    		if(contactList[contact.index].size - user.index > 1)
    				    		{
    				    			for(int j = user.index; j < contactList[contact.index].size-1; j++)
    				    		    {
    				    			 // Copy user left 1
    				    		     strcpy(contactList[contact.index].userList[j].contactName, contactList[contact.index].userList[j+1].contactName);
    				    		     strcpy(contactList[contact.index].userList[j].IP, contactList[contact.index].userList[j+1].IP);
    				    		     contactList[contact.index].userList[j].port = contactList[contact.index].userList[j+1].port;

    				    		     // Delete original user
    				    		     strcpy(contactList[contact.index].userList[j+1].contactName, "");
    				    		     strcpy(contactList[contact.index].userList[j+1].IP, "");
    				    		     contactList[contact.index].userList[j+1].port = 0;
    				    		     }
    				    		}

    				    		contactList[contact.index].size--; // Removed contact
    				    		printf("%s contact list size updated to: %d\n\n", contactList[contact.index].listName, contactList[contact.index].size);
    				    		
    				    		//Update return code
    				    		strcpy(data.returnCode, "SUCCESS");
    				    	}
    				    	else
    				    	{
    				    		printf("Error: User is not on the list. \n\n");
    				    		//Update return code
    				    		strcpy(data.returnCode, "FAILURE");
    				    	}
    				    }
    				}
    				else
    				{
    				    printf("Error: Contact name or contact list not found.\n\n");

    				    // Update return code
    				    strcpy(data.returnCode, "FAILURE");
    				}
    			}
    			else
    			{
    				printf("Error: No users or contact lists registered.\n\n");

    			    // Update return code
    			    strcpy(data.returnCode, "FAILURE");
    			}
    			break; 

    		case 5: // exit messaging
    			printf("Server received exit command. contactName: %s\n\n", data.contactName);

    			// Checking if there's a user to remove
    			if(userNum > 0)
    			{
    				user = searchUser(userList, userNum, data.contactName);

    				// Checking if user name exists
    				if(user.found == 1)
    				{
    					// Remove user from register list
    					printf("%s was removed from the register.\n", data.contactName);

    					// Remove user from list
    					strcpy(userList[user.index].contactName, "");
    					strcpy(userList[user.index].IP, "");
    					userList[user.index].port = 0;

    					// Rebuild list
    					if(userNum - user.index > 1)
    					{
    						for(int i = user.index; i < userNum-1; i++)
    						{
    							// Copy user left 1
    							strcpy(userList[i].contactName, userList[i+1].contactName);
    							strcpy(userList[i].IP, userList[i+1].IP);
    							userList[i].port = userList[i+1].port;

    							// Delete original user
    							strcpy(userList[i+1].contactName, "");
    							strcpy(userList[i+1].IP, "");
    							userList[i+1].port = 0;
    						}
    					}

    					userNum--; // Removed user
    					printf("User count updated to: %d\n\n", userNum);

    					// Remove user from all contact lists
            			for(int i = 0; i < listNum; i++)
            			{
            				user = searchUser(contactList[i].userList, contactList[i].size, data.contactName);

            				if(user.found == 1)
            				{
            					printf("%s was removed from the %s contact list.\n", data.contactName, contactList[i].listName);

            					// Remove user from list
            					strcpy(contactList[i].userList[user.index].contactName, "");
            					strcpy(contactList[i].userList[user.index].IP, "");
            					contactList[i].userList[user.index].port = 0;

            					// Rebuild list
            					if(contactList[i].size - user.index > 1)
            					{
            						for(int j = user.index; j < contactList[i].size-1; j++)
            						{
            							// Copy user left 1
                    					strcpy(contactList[i].userList[j].contactName, contactList[i].userList[j+1].contactName);
                    					strcpy(contactList[i].userList[j].IP, contactList[i].userList[j+1].IP);
                    					contactList[i].userList[j].port = contactList[i].userList[j+1].port;

                    					// Delete original user
                    					strcpy(contactList[i].userList[j+1].contactName, "");
                    					strcpy(contactList[i].userList[j+1].IP, "");
                    					contactList[i].userList[j+1].port = 0;
            						}
            					}

            					contactList[i].size--; // Removed contact
            					printf("%s contact list size updated to: %d\n\n", contactList[i].listName, contactList[i].size);
            				}
            			}

                		// Update return code
                		strcpy(data.returnCode, "SUCCESS");
    				}
    				else
    				{
    					printf("Error: Contact name not found.\n\n");

                		// Update return code
                		strcpy(data.returnCode, "FAILURE");
    				}
    			}
    			else
    			{
    				printf("Error: No users registered.\n\n");

            		// Update return code
            		strcpy(data.returnCode, "FAILURE");
    			}

    			break;
    			
    		case 6: //im-start
    			printf("Server recieved im-start command. \n\n");
    			if(listNum > 0 && userNum > 0)
    			{
    			  user = searchUser(userList, userNum, data.contactName);
    			  contact = searchList(contactList, listNum, data.listName);
    			  
    			  if(user.found == 1 && contact.found == 1)
    			  {
    				  size = contactList[contact.index].size;
    				  //checking to see if list is empty
    				  if(size == 0)
    				  {
    					  printf("Error: No users in the contact lists registered.\n\n");
    				      // Update return code
    				      strcpy(data.returnCode, "FAILURE");
    				  }
    				  else if(size == 1)
    				  {
    					  printf("Error: Only 1 user in the contact lists registered.\n\n");
    					  strcpy(data.returnCode, "FAILURE");
    					  
    				  }
    				  else
    				  {
    					  user = searchUser(contactList[contact.index].userList, contactList[contact.index].size, data.contactName);
    					  /*
    					   *  Further code to be added 
    					   *  strcpy(data.returnCode, "SUCCESS");
    					   */
    					  strcpy(data.returnCode, "FAILURE");
    				  }
    			  }
    			  else
    			  {
    				  printf("Error: Contact name or contact list not found.\n\n");
    				  
    				  // Update return code
    				  strcpy(data.returnCode, "FAILURE");
    			  }
    			}
    			else
    			{
    				printf("Error: No users registered.\n\n");

    				// Update return code
    				strcpy(data.returnCode, "FAILURE"); 				
    			}
    			
    			break;
    			
    		case 7: //im-complete
    			/*
    			 * Somehow, we need to be able to check to see if the im-start command was activated
    			 * If it was not activated, then, this commands returns a failure. 
    			 */
    			printf("Server recieved im-complete command. \n\n");
    			printf("Error \n\n");
    			strcpy(data.returnCode, "FAILURE");
    			break;

    		case 8: // save contacts
    			printf("Server received save command. fileName: %s\n\n", data.fileName);

				// Build string with .txt added
				sprintf(fileName, "%s.txt", data.fileName);

				// Open file for output
    			FILE *file;
    			file = fopen(fileName, "w");

    			// Write active users
    			fprintf(file, "There are %d users in the register.\n\n", userNum);

    			// Write contact info
    			for(int i = 0; i < userNum; i++)
    			{
    				fprintf(file, "%d: Contact name: %s, IP: %s, port: %hu\n", i+1, userList[i].contactName, userList[i].IP, userList[i].port);
    			}

    			fprintf(file, "\n"); // Spacer

    			// Write contact lists
    			fprintf(file, "There are %d contact lists.\n\n", listNum);

    			// Write contact list info
    			for(int i = 0; i < listNum; i++)
    			{
    				fprintf(file, "%d: Contact list name: %s\n", i+1, contactList[i].listName);

	    			// Write list users
	    			fprintf(file, "There are %d users in this list.\n\n", contactList[i].size);

    				for(int j = 0; j < contactList[i].size; j++)
    				{
    					fprintf(file, "%d: Contact name: %s, IP: %s, port: %hu\n", j+1, contactList[i].userList[j].contactName, contactList[i].userList[j].IP, contactList[i].userList[j].port);
    				}

    				fprintf(file, "\n"); // Spacer
    			}

    			// Close file
    			if(fclose(file) == 0)
    			{
    				printf("%s saved.\n\n", fileName);

            		// Update return code
            		strcpy(data.returnCode, "SUCCESS");
    			}
    			else
    			{
    				printf("Error: File failed to close.\n\n");

            		// Update return code
            		strcpy(data.returnCode, "FAILURE");
    			}

    			break;

    		default:
    			printf("Error: invalid command received.\n\n");
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

