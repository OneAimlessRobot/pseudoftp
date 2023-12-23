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
	int fd=acessVarMtx(&varMtx,&nextClient->fd,0,-1);
	u_int64_t dataSize=acessVarMtx(&varMtx,&state->dataSize,0,-1);
	u_int16_t pingSize=(u_int16_t)acessVarMtx(&varMtx,&state->pingSize,0,-1);
	char message[dataSize];
	memset(message,0,dataSize);
	int client_socket=acessVarMtx(&varMtx,&nextClient->client_socket,0,-1);
	u_int64_t client_total=acessVarMtx(&varMtx,&nextClient->bytesToRead,0,-1);

	while(acessVarMtx(&varMtx,&state->serverRunning,0,-1)&&!acessVarMtx(&varMtx,&nextClient->done,0,-1)){
	
	int numRead=read(fd,message,dataSize);
	int toSend=notifyClientAboutSizes(nextClient);
	//s r	
	if(toSend>0){
			char buff3[LOGMSGLENGTH]={0};
			snprintf(buff3,LOGMSGLENGTH,"Sending chunk of data to %s!!!!",inet_ntoa(nextClient->clientAddress.sin_addr));
			pushLog(buff3);
			u_int64_t numSent=send(client_socket,message,numRead,0);
			char pingBuff[pingSize];
			receiveClientPing(nextClient,pingBuff,pingSize); 
		acessVarMtx(&varMtx,&state->totalSent,numSent,3);
		u_int64_t clientCurrTotal=acessVarMtx(&varMtx,&nextClient->numOfBytesSent,numSent,3);
		u_int64_t clientGoal=acessVarMtx(&varMtx,&nextClient->bytesToRead,0,-1);
		if(clientCurrTotal==clientGoal){
		
			
			acessVarMtx(&varMtx,&nextClient->done,1,0);
			acessStackMtx(&stackMtx,state->kickedClients,nextClient,0);

	}
	}
	else{
		
			acessVarMtx(&varMtx,&nextClient->done,1,0);
			acessStackMtx(&stackMtx,state->kickedClients,nextClient,0);

	}
	}
	pthread_cond_signal(&kickingCond);
	

	char buff5[LOGMSGLENGTH]={0};
	snprintf(buff5,LOGMSGLENGTH,"Tudo enviado!!!!");
	pushLog(buff5);
	return NULL;
}
static int processClientInfo(clientStruct* currClient){
//printf("cheguei!!!\n");
u_int16_t pingSize=(u_int16_t)acessVarMtx(&varMtx,&state->pingSize,0,-1);
u_int64_t dataSize=acessVarMtx(&varMtx,&state->dataSize,0,-1);
char ping[pingSize];
char userPrompt[pingSize];
char passPrompt[pingSize];
//printf("cheguei!!!\n");
memset(ping,0,pingSize);
memset(userPrompt,0,pingSize);
memset(passPrompt,0,pingSize);
int client_socket=(int)acessVarMtx(&varMtx,&currClient->client_socket,0,-1);
int fd=(int)acessVarMtx(&varMtx,&currClient->fd,0,-1);
		snprintf(ping,pingSize,"%u %lu %s",pingSize, dataSize,pingCorrect);
		strcpy(userPrompt,userNamePrompt);
		strcpy(passPrompt,passWordPrompt);
		send(client_socket,ping,pingSize,0);
		//printf("cheguei!!!\n");
		memset(ping,0,pingSize);
		receiveClientPing(currClient,ping,pingSize);
		char buff3[LOGMSGLENGTH]={0};
		snprintf(buff3,LOGMSGLENGTH,"client got the sizes!!!!!!!");
		pushLog(buff3);
		loginStruct login,*storedClient;
		memset(login.user,0,FIELDLENGTH+1);
		memset(login.password,0,FIELDLENGTH+1);
		send(client_socket,userPrompt,pingSize,0);
		receiveClientField(currClient,login.user,pingSize);
		send(client_socket,passPrompt,pingSize,0);
		receiveClientField(currClient,login.password,pingSize);
		//s r
//printf("cheguei!!!\n");
		if((storedClient=(loginStruct*)getHTElemComp(loadedLogins,&login))){
			
			
			if(!strncmp(login.password,storedClient->password,pingSize)){
			int size=lseek(fd,0,SEEK_END);
			lseek(fd,0,SEEK_SET);
			acessVarMtx(&varMtx,&currClient->bytesToRead,size,0);
			acessVarMtx(&varMtx,&currClient->isAdmin,0,0);
			memset(currClient->login,0,FIELDLENGTH+1);
			strncpy(currClient->login,storedClient->user,FIELDLENGTH);
			acessVarMtx(&varMtx,&state->clientsActive,0,1);
			acessListMtx(&listMtx,state->listOfClients,currClient,0,0);
			char buff[pingSize];
			memset(buff,0,pingSize);
			snprintf(buff,pingSize,"Bem vindo, %s!\n",login.user);
			send(client_socket,buff,pingSize,0);
			pthread_create(&currClient->threadid,NULL,dataSending,currClient);
			return 1;
			}
			else{
			
			char buff3[LOGMSGLENGTH]={0};
			snprintf(buff3,LOGMSGLENGTH,"client kicked!!!!!!!! Credentials incorrect... No one with username %s and password %s exists!!!!\n(O user %s tem password %s)\n",login.user,login.password,login.user,storedClient->password);
			pushLog(buff3);
			close(client_socket);
			return 0;
			}
			
		}
		else if(!strncmp(login.user,pingAdmin,strlen(pingAdmin))){
			
			char buff3[LOGMSGLENGTH]={0};
			snprintf(buff3,LOGMSGLENGTH,"client got the sizes!!!!!!!");
			pushLog(buff3);
			char buff4[LOGMSGLENGTH]={0};
			snprintf(buff4,LOGMSGLENGTH,"ADMIN HAS JOINED!!!!!");
			pushLog(buff4);
			memset(currClient->login,0,FIELDLENGTH+1);
			acessVarMtx(&varMtx,&currClient->isAdmin,1,0);
			strncpy(currClient->login,pingAdmin,FIELDLENGTH);
			acessVarMtx(&varMtx,&state->adminsActive,0,1);
			acessListMtx(&listMtx,state->listOfAdmins,currClient,0,0);
			send(client_socket,welcomeAdmin,LINESIZE,0);
			pthread_create(&currClient->threadid,NULL,enqueueCmds,currClient);
			return 2;
		}
		else{

			char buff3[LOGMSGLENGTH]={0};
			snprintf(buff3,LOGMSGLENGTH,"client kicked!!!!!!!! Credentials given were %s and password %s\n",login.user,login.password,strlen(login.user));
			pushLog(buff3);
			close(client_socket);
			return 0;
		}

}
static void* connectionAccepting(void* argStruct){
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
		tv.tv_sec=MAXTIMEOUTSECS;
		tv.tv_usec=MAXTIMEOUTUSECS;
		pushLog("A espera de conexoes!!!");
		iResult=select(state->server_socket+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
		if(iResult>0){
		clientStruct currClient;
		memset(&currClient,0,sizeof(clientStruct));
		memset(&currClient.clientAddress,0,sizeof(struct sockaddr));
		memset(&currClient.addrLength,0,sizeof(socklen_t));
		currClient.numOfBytesSent=0;
		currClient.done=0;
		currClient.isAdmin=0;
		currClient.client_socket=accept(state->server_socket,(struct sockaddr*)&(currClient.clientAddress),&(currClient.addrLength));
		if((currClient.fd=open((char*)acessVarMtx(&varMtx,&state->pathToFile,0,-1),O_RDONLY,0666))<0){

			char buff[LOGMSGLENGTH]={0};
			snprintf(buff,LOGMSGLENGTH,"Accepted connection from %s, mas ficheiro %s e invalido. Conexao sera largada...",inet_ntoa(currClient.clientAddress.sin_addr),acessVarMtx(&varMtx,&state->pathToFile,0,-1));
			pushLog(buff);
			perror("Nao foi possivel abrir nada!!!!\n");
			close(currClient.client_socket);
		
		}
		else{
			char buff[LOGMSGLENGTH]={0};
			snprintf(buff,LOGMSGLENGTH,"Accepted connection from %s",inet_ntoa(currClient.clientAddress.sin_addr));
			pushLog(buff);
			processClientInfo(&currClient);
			
		}
			
		}
		else{
			char buff[LOGMSGLENGTH]={0};
			snprintf(buff,LOGMSGLENGTH,"Timed out!!!!!( more that %ds waiting). Trying again...",MAXTIMEOUTSECS);
			pushLog(buff);
		}
	}
	
	acessVarMtx(&varMtx,&state->idle,0,0);
		
	pushLog("Fechou a loja!!!!");
	}
	return NULL;


}
void initEverything(u_int16_t port,char*pathToFile,u_int16_t startPingSize,u_int64_t startDataSize,u_int16_t startMaxNumOfClients){
	
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
	state->pingSize=startPingSize;
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
