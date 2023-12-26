#ifndef SERVER_H
#define SERVER_H


#define LOGMSGLENGTH 100
#define PINGSIZE 50
#define OUTPUTLENGTH 100000
#define MAXTIMEOUTSECS 1
#define MAXTIMEOUTUSECS 0
#define MAXTIMEOUTPING 1
#define MAXTIMEOUTPINGU 0
#define MAXIDLECLIENT 5
#define MAX_CLIENTS_HARD_LIMIT 1000

typedef struct serverState{
	
	char logBuff[LOGMSGLENGTH*100];
	char addressContainer[INET_ADDRSTRLEN];
	char *pathToFile;
	u_int64_t serverRunning;
	u_int64_t idle;
	u_int64_t adminsActive,clientsActive,maxNumOfClients;
	DListW* listOfClients;
	DListW* listOfAdmins;
	stackList* logs,*kickedClients;
	int server_socket;
	struct sockaddr_in server_address;
	u_int64_t totalSent;
	u_int64_t timeActive;
	double trafficRate;
	u_int64_t dataSize;
	
}serverState;

typedef struct clientStruct{
	pthread_t threadid;
	struct	sockaddr_in clientAddress;
	int client_socket;
	socklen_t addrLength;
	u_int64_t numOfBytesSent;
	u_int64_t done;
	u_int64_t bytesToRead;
	int fd;
	u_int64_t isAdmin;
	char login[FIELDLENGTH+1];
}clientStruct;

typedef struct command{

	char string[LINESIZE];
	clientStruct* whoItWas;


}command;
void initEverything(u_int16_t port,char* pathToFile,u_int64_t startDataSize,u_int16_t startMaxNumOfClients);

void sigint_handler(int signal);

void cleanup(void);

#endif

