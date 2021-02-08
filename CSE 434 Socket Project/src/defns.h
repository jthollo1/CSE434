struct regUser
{
	char contactName[50];
	char IP[20];
	unsigned short port;
	char returnCode[10];
};

struct createList
{
	char listName[50];
	char returnCode[10];
};

struct queryList
{
	char list[50][50];
	char returnCode[10];
};

struct joinList
{
	char listName[50];
	char contactName[50];
	char returnCode[10];
};

struct exitP
{
	char contactName[50];
	char returnCode[10];
};

struct saveFile
{
	char fileName[50];
	char returnCode[10];
};
