#include "../Includes/preprocessor.h"
#include "../cmdModules/Includes/servercmds.h"
#include "../Includes/hud.h"
#include "../Includes/login.h"
#include "../Includes/client.h"
#include "../Includes/admin.h"
#include "../Includes/infometers.h"
#include "../Includes/accounts.h"
const char* pingCorrect= "queroja";
const char* pingAdmin= "admin";

static const char* welcomeAdmin= "Bem vindo, admin!\nPara ajuda, mete 'sh' para veres uma lista de comandos disponiveis!!!!!\n";

serverState* state=NULL;
hashtablecomp* loadedLogins=NULL;
const char*  userNamePrompt="username> ";
const char* passWordPrompt="password> ";
const char* logFile="./logs.log";
const char* loginsFile="./logins.dat";
int loginfd, logfd,logging=1;
static pthread_mutex_t acceptingMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t kickingMtx= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t varMtx= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t listMtx= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stackMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t exitingMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t graphicsMtx= PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t acceptingCond= PTHREAD_COND_INITIALIZER;
pthread_cond_t kickingCond= PTHREAD_COND_INITIALIZER;
static pthread_t connectionAccepter,dataSender,graphics,rateInfoUpdater,clientKicker,serverLogWritter;



void sigpipe_handler(int signal){




}

void cleanup(void){
	
	pthread_join(rateInfoUpdater,NULL);
	if(logging){
	pthread_join(serverLogWritter,NULL);
	}
	pthread_join(clientKicker,NULL);
	pthread_join(graphics,NULL);
	pthread_join(connectionAccepter,NULL);
	acessListMtx(&listMtx,state->listOfClients,NULL,0,3);
	acessListMtx(&listMtx,state->listOfAdmins,NULL,0,3);
	acessStackMtx(&listMtx,state->logs,NULL,2);
	acessStackMtx(&listMtx,state->kickedClients,NULL,2);
	close(state->server_socket);
	saveLogins();
	if(loadedLogins){

		destroyHashTableComp(loadedLogins);
	}
	free(state);
	nocbreak();
	endwin();
	printf("server a fechar!!!\n");
	

}

void sigint_handler(int signal){


	acessVarMtx(&varMtx,&state->serverRunning,0,0);
	acessVarMtx(&varMtx,&state->idle,0,0);
	pthread_cond_signal(&kickingCond);
	pthread_cond_signal(&acceptingCond);
	
}

static void* clientKickingFunc(void* args){

	while(acessVarMtx(&varMtx,&state->serverRunning,0,-1)){
			
		pthread_mutex_lock(&kickingMtx);
		while(!(*(u_int64_t*)acessStackMtx(&stackMtx,state->kickedClients,0,3))&&acessVarMtx(&varMtx,&state->serverRunning,0,-1)){
			
			
			pthread_cond_wait(&kickingCond,&kickingMtx);

		}
		pthread_mutex_unlock(&kickingMtx);
		
		while((*(u_int64_t*)acessStackMtx(&stackMtx,state->kickedClients,0,3))&&acessVarMtx(&varMtx,&state->serverRunning,0,-1)){
			clientStruct* nextClient= (clientStruct*)acessStackMtx(&stackMtx,state->kickedClients,0,1);
			//printf("Cliente: %p Numero de clients: %lu\n",nextClient,state->clientsActive);
			
	
				close((int)acessVarMtx(&varMtx,&nextClient->client_socket,0,-1));
				close((int)acessVarMtx(&varMtx,&nextClient->fd,0,-1));
				pthread_t thethread=(pthread_t) acessVarMtx(&varMtx,&nextClient->threadid,0,-1);
				pthread_join(thethread,NULL);
				if(!acessVarMtx(&varMtx,&nextClient->isAdmin,0,-1)){

				
					acessListMtx(&listMtx,state->listOfClients,nextClient,0,1);
					acessVarMtx(&varMtx,&state->clientsActive,0,2);
					

				}
				else{

					acessListMtx(&listMtx,state->listOfAdmins,nextClient,0,1);
					acessVarMtx(&varMtx,&state->adminsActive,0,2);
					
				}
				free(nextClient);
		}
		pthread_cond_signal(&acceptingCond);
		
		
	}




}


static void* enqueueCmds(void* argStruct){
	clientStruct* nextClient= (clientStruct*) argStruct;
	int client_socket=acessVarMtx(&varMtx,&nextClient->client_socket,0,-1);
	hashtablecomp* cmdLine=initCmdLine(servercmds);

	while(acessVarMtx(&varMtx,&state->serverRunning,0,-1)&&!acessVarMtx(&varMtx,&nextClient->done,0,-1)){
	char input[LINESIZE];
	memset(input,0,LINESIZE);
	int result=receiveClientCommand(nextClient,input);
	if(result>0){
			
			command cmd;
			memset(cmd.string,0,LINESIZE);
			strncpy(cmd.string,input,LINESIZE);
			char buff[LOGMSGLENGTH];
			snprintf(buff,LOGMSGLENGTH,"Comando executado!!!! %s\n",cmd.string);
			pushLog(buff);
			cmd.whoItWas=nextClient;
			char outBuff[LINESIZE]={0};
			runCmdLine(cmdLine,cmd.whoItWas,cmd.string,outBuff);
			send((int)acessVarMtx(&varMtx,&cmd.whoItWas->client_socket,0,-1),outBuff,LINESIZE,0);

	}
	}
	return NULL;
	destroyHashTableComp(cmdLine);

}

static void* dataSending(void* argStruct){
	clientStruct* nextClient= (clientStruct*) argStruct;
	
	u_int64_t dataSize=acessVarMtx(&varMtx,&state->dataSize,0,-1),
		client_total=nextClient->bytesToRead;
	char message[dataSize],
		buff3[LOGMSGLENGTH]={0},
		pingBuff[PINGSIZE]={0};
	int client_socket=nextClient->client_socket,
			fd=nextClient->fd;
	while(acessVarMtx(&varMtx,&state->serverRunning,0,-1)&&!nextClient->done){
	
	int numRead=read(fd,message,dataSize),
		toSend=notifyClientAboutSizes(nextClient,numRead);
	if(toSend>0){
			snprintf(buff3,LOGMSGLENGTH,"Sending chunk of data to %s!!!!",inet_ntoa(nextClient->clientAddress.sin_addr));
			pushLog(buff3);
			send(client_socket,message,numRead,0);
			receiveWholeClientPing(nextClient,pingBuff,PINGSIZE);
			int numSent=-1;
			sscanf(pingBuff,"%d",&numSent);
			
			
			
		acessVarMtx(&varMtx,&state->totalSent,numSent,3);
		u_int64_t clientCurrTotal=nextClient->numOfBytesSent+=numSent;
		if(clientCurrTotal==client_total){
		
			
			nextClient->done=1;
			acessStackMtx(&stackMtx,state->kickedClients,nextClient,0);

	}
	}
	else{
		
			nextClient->done=1;
			acessStackMtx(&stackMtx,state->kickedClients,nextClient,0);

	}
	memset(buff3,0,LOGMSGLENGTH);
	}
	pthread_cond_signal(&kickingCond);
	
	snprintf(buff3,LOGMSGLENGTH,"Tudo enviado!!!!");
	pushLog(buff3);
	return NULL;
}
static int processClientInfo(clientStruct* currClient){
u_int64_t dataSize=acessVarMtx(&varMtx,&state->dataSize,0,-1);
char ping[PINGSIZE]={0},
	userPrompt[FIELDLENGTH+1]={0},
	passPrompt[FIELDLENGTH+1]={0},
	buff3[LOGMSGLENGTH]={0};
	loginStruct login,*storedClient;
	memset(login.user,0,FIELDLENGTH+1);
	memset(login.password,0,FIELDLENGTH+1);
				
	strcpy(userPrompt,userNamePrompt);
	strcpy(passPrompt,passWordPrompt);

		int fd=(int)acessVarMtx(&varMtx,&currClient->fd,0,-1);
		snprintf(ping,PINGSIZE,"%lu", dataSize);
		send(currClient->client_socket,ping,PINGSIZE,0);
		memset(ping,0,PINGSIZE);
		receiveWholeClientPing(currClient,ping,PINGSIZE);
		memset(buff3,0,LOGMSGLENGTH);
		snprintf(buff3,LOGMSGLENGTH,"client got the sizes!!!!!!!");
		pushLog(buff3);
		send(currClient->client_socket,userPrompt,FIELDLENGTH+1,0);
		receiveWholeClientPing(currClient,login.user,FIELDLENGTH+1);
		send(currClient->client_socket,passPrompt,FIELDLENGTH+1,0);
		receiveWholeClientPing(currClient,login.password,FIELDLENGTH+1);
		memset(buff3,0,LOGMSGLENGTH);
		if((storedClient=(loginStruct*)getHTElemComp(loadedLogins,&login))){

			if(!strncmp(login.password,storedClient->password,PINGSIZE)){
			int size=lseek(fd,0,SEEK_END);
			lseek(fd,0,SEEK_SET);
			acessVarMtx(&varMtx,&currClient->bytesToRead,size,0);
			acessVarMtx(&varMtx,&currClient->isAdmin,0,0);
			memset(currClient->login,0,FIELDLENGTH+1);
			strncpy(currClient->login,storedClient->user,FIELDLENGTH);
			acessVarMtx(&varMtx,&state->clientsActive,0,1);
			acessListMtx(&listMtx,state->listOfClients,currClient,0,0);
			memset(ping,0,PINGSIZE);
			snprintf(ping,PINGSIZE,"Bem vindo, %s!\n",login.user);
			send(currClient->client_socket,ping,PINGSIZE,0);
			pthread_create(&currClient->threadid,NULL,dataSending,currClient);
			return 1;
			}
			else{
			
			snprintf(buff3,LOGMSGLENGTH,"client kicked!!!!!!!! Credentials incorrect... No one with username %s and password %s exists!!!!\n(O user %s tem password %s)\n",login.user,login.password,login.user,storedClient->password);
			pushLog(buff3);
			close(currClient->client_socket);
			return 0;
			}
			
		}
		else if(!strncmp(login.user,pingAdmin,strlen(pingAdmin))){
			
			snprintf(buff3,LOGMSGLENGTH,"client got the sizes!!!!!!!");
			pushLog(buff3);
			memset(buff3,0,LOGMSGLENGTH);
			snprintf(buff3,LOGMSGLENGTH,"ADMIN HAS JOINED!!!!!");
			pushLog(buff3);
			acessVarMtx(&varMtx,&currClient->isAdmin,1,0);
			strncpy(currClient->login,pingAdmin,FIELDLENGTH);
			acessVarMtx(&varMtx,&state->adminsActive,0,1);
			acessListMtx(&listMtx,state->listOfAdmins,currClient,0,0);
			send(currClient->client_socket,welcomeAdmin,LINESIZE,0);
			pthread_create(&currClient->threadid,NULL,enqueueCmds,currClient);
			return 2;
		}
		else{

			snprintf(buff3,LOGMSGLENGTH,"client kicked!!!!!!!! Credentials given were %s and password %s\n",login.user,login.password,strlen(login.user));
			pushLog(buff3);
			close(currClient->client_socket);
			return 0;
		}

}
static void* connectionAccepting(void* argStruct){
	char buff[LOGMSGLENGTH]={0};
			
	while(acessVarMtx(&varMtx,&state->serverRunning,0,-1)){
	

		
	pthread_mutex_lock(&acceptingMtx);
	while(acessVarMtx(&varMtx,&state->clientsActive,0,-1)>=acessVarMtx(&varMtx,&state->maxNumOfClients,0,-1)&&acessVarMtx(&varMtx,&state->serverRunning,0,-1)){

		pthread_cond_wait(&acceptingCond,&acceptingMtx);
	}
	pthread_mutex_unlock(&acceptingMtx);
	if(!acessVarMtx(&varMtx,&state->serverRunning,0,-1)){


		return NULL;
	}
	
		while(acessVarMtx(&varMtx,&state->clientsActive,0,-1)<acessVarMtx(&varMtx,&state->maxNumOfClients,0,-1)&&acessVarMtx(&varMtx,&state->serverRunning,0,-1)){
		
		int iResult;
		struct timeval tv;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(state->server_socket,&rfds);
		tv.tv_sec=MAXTIMEOUTCONSECS;
		tv.tv_usec=MAXTIMEOUTCONUSECS;
		pushLog("A espera de conexoes!!!");
		iResult=select(state->server_socket+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
		if(iResult>0){
		clientStruct currClient;
		//memset(&currClient,0,sizeof(clientStruct));
		//memset(&currClient.clientAddress,0,sizeof(struct sockaddr));
		//memset(&currClient.addrLength,0,sizeof(socklen_t));
		memset(currClient.login,0,FIELDLENGTH+1);
		currClient.numOfBytesSent=0;
		currClient.done=0;
		currClient.isAdmin=0;
		currClient.client_socket=accept(state->server_socket,(struct sockaddr*)&(currClient.clientAddress),&(currClient.addrLength));
		memset(buff,0,LOGMSGLENGTH);
		if((currClient.fd=open((char*)acessVarMtx(&varMtx,&state->pathToFile,0,-1),O_RDONLY,0666))<0){

			snprintf(buff,LOGMSGLENGTH,"Accepted connection from %s, mas ficheiro %s e invalido. Conexao sera largada...",inet_ntoa(currClient.clientAddress.sin_addr),(char*)acessVarMtx(&varMtx,&state->pathToFile,0,-1));
			pushLog(buff);
			perror("Nao foi possivel abrir nada!!!!\n");
			close(currClient.client_socket);
		
		}
		else{
			snprintf(buff,LOGMSGLENGTH,"Accepted connection from %s",inet_ntoa(currClient.clientAddress.sin_addr));
			pushLog(buff);
			processClientInfo(&currClient);
			
		}
			
		}
		else{
			snprintf(buff,LOGMSGLENGTH,"Timed out!!!!!( more that %ds waiting). Trying again...",MAXTIMEOUTCONSECS);
			pushLog(buff);
		}
	}
	
	acessVarMtx(&varMtx,&state->idle,0,0);
		
	pushLog("Fechou a loja!!!!");
	}
	return NULL;


}
void initEverything(u_int16_t port,char*pathToFile,u_int64_t startDataSize,u_int16_t startMaxNumOfClients){
	
	loadLogins();
	logfd=creat(logFile,0777);
	if(logfd<0){

		perror("Nao foi possivel abrir o ficheiro de logs!!!!( Nao vais ver logs!!!)");
		logging=0;
	}	
	state=malloc(sizeof(serverState));
	
	state->server_socket= socket(AF_INET,SOCK_STREAM,0);
	int ptr=1;
	setsockopt(state->server_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&ptr,sizeof(ptr));
	if(state->server_socket==-1){
		free(state);
		return;

	}
	
	//especificar socket;
	fcntl(state->server_socket,F_SETFD,O_ASYNC);
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigint_handler);
	state->server_address.sin_family=AF_INET;
	state->server_address.sin_port= htons(port);
		
	state->server_address.sin_addr.s_addr = INADDR_ANY;
	bind(state->server_socket,(struct sockaddr*) &state->server_address,sizeof(state->server_address));
	
	state->maxNumOfClients= startMaxNumOfClients;
	
	listen(state->server_socket,MAX_CLIENTS_HARD_LIMIT);
	
	struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&state->server_address;
	struct in_addr ipAddr = pV4Addr->sin_addr;
	inet_ntop( AF_INET, &ipAddr, state->addressContainer, INET_ADDRSTRLEN );
	
	
        state->totalSent=state->timeActive=0;
        state->trafficRate=0.0;
	state->pathToFile=pathToFile;
	state->serverRunning=1;
	state->idle=1;
	state->adminsActive=0;
	state->clientsActive=0;
	state->listOfClients=initDList(sizeof(clientStruct));
	state->listOfClients->head=NULL;
	state->listOfAdmins=initDList(sizeof(clientStruct));
	state->listOfAdmins->head=NULL;
	state->logs=initDLStack(LOGMSGLENGTH);
	state->kickedClients=initDLStack(sizeof(clientStruct));
	state->dataSize=startDataSize;
	
	initscr();
	start_color();
	noecho();
	pthread_create(&graphics,NULL,graphicsLoop,NULL);
	
	
	pthread_create(&connectionAccepter,NULL,connectionAccepting,NULL);
	pthread_create(&rateInfoUpdater,NULL,updateRates,NULL);
	pthread_create(&clientKicker,NULL,clientKickingFunc,NULL);
	if(logging){
	pthread_create(&serverLogWritter,NULL,serverLogWritting,NULL);
	}
}
