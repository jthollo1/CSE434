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

struct foundUser
{
	char contactName[50];      // client contact name
	char IP[20];               // IP address
	unsigned short port;       // port number
	int index;                 // index user was found
	int found;                 // found variable
};

struct user
{
	char contactName[50];      // client contact name
	char IP[20];               // IP address
	unsigned short port;       // port number
};

struct cList
{
	char listName[50];
	struct user userList[50];
	int size;
};

struct foundContact
{
	int index;
	int found;
};
