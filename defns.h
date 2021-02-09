struct dataStruct
{
	int command;               // command code: 1 = register, 2 = create, 3 = query, 4 = join, 5 = exit, 6 = save

	char listName[50];         // contact list name
	char contactName[50];      // client contact name
	char contactList[50][50];  // list of contact names

	char IP[20];               // IP address
	unsigned short port;       // port number

	char fileName[50];         // save file name

	char returnCode[10];       // return code
};
