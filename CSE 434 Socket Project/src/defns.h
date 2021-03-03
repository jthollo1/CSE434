struct user
{
	char contactName[50];      // client contact name
	char IP[20];               // IP address
	unsigned short port;       // port number
};

struct cList
{
	char listName[50];         // contact list name
	struct user userList[50];  // list of users
	int size;                  // contact list size
	int im_running;            // IM running flag
};

struct serverStruct
{
	unsigned short port;       // port number
};

struct dataStruct
{
	int command;               // command code: 0 = register, 1 = create, 2 = query, 3 = join, 4 = leave, 5 = exit, 6 = im-start, 7 = im-complete, 8 = save

	char listName[50];         // contact list name
	char contactName[50];      // client contact name
	char contactLists[50][50]; // list of contact lists
	struct user userList[50];  // list of users

	char IP[20];               // IP address
	unsigned short port;       // port number

	char fileName[50];         // save file name

	char returnCode[10];       // return code
};

struct msgStruct
{
	char message[200];         // message
	struct user userList[50];  // list of users
	int size;                  // contact list size

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

struct foundContact
{
	int index;                 // index contact list was found
	int found;                 // found variable
};
