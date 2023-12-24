#include "../Includes/preprocessor.h"
#include "../Includes/hud.h"

extern serverState* state;
extern pthread_mutex_t stackMtx;
extern pthread_mutex_t varMtx;
extern pthread_mutex_t listMtx;
extern int logfd;
static void formatCurrTime(char buff[LOGMSGLENGTH]){

	time_t timer;
	struct tm* tm_info;
	timer= time(NULL);
	tm_info= localtime(&timer);
	strftime(buff,LOGMSGLENGTH,"%Y-%m-%d %H:%M:%S", tm_info);
	

}
void printClientsOfState(char buff[LOGMSGLENGTH*20]){
	
	char* curr=buff;
	for(u_int64_t i=0;i<acessVarMtx(&varMtx,&state->clientsActive,0,-1);i++){
		
		clientStruct* currClient=(clientStruct*)acessListMtx(&listMtx,state->listOfClients,0,i,2);
		
		curr+=snprintf(curr,LOGMSGLENGTH*10,"Client number %lu: %s\nLogin: %s\nLiga-nos da porta: %d\nTem numero de socket: %d\nJa recebeu %lu bytes:\nJa foi despachado???????%lu\n",i,inet_ntoa(currClient->clientAddress.sin_addr),currClient->login,(int) ntohs(currClient->clientAddress.sin_port),(int32_t)acessVarMtx(&varMtx,&currClient->client_socket,0,-1),acessVarMtx(&varMtx,&currClient->numOfBytesSent,0,-1),acessVarMtx(&varMtx,&currClient->done,0,-1));
	}
}	

void printAdminsOfState(char buff[LOGMSGLENGTH*20]){
	
	char* curr=buff;
	for(u_int64_t i=0;i<*(u_int64_t*)acessListMtx(&listMtx,state->listOfAdmins,0,0,4);i++){
		
		clientStruct* currClient=(clientStruct*)acessListMtx(&listMtx,state->listOfAdmins,0,i,2);
		curr+=snprintf(curr,LOGMSGLENGTH,"Admin number %lu: %s.\nLiga-nos da porta: %d\nTem numero de socket: %d\n\n",i,inet_ntoa(currClient->clientAddress.sin_addr),(int) ntohs(currClient->clientAddress.sin_port),(int32_t)acessVarMtx(&varMtx,&currClient->client_socket,0,-1));
		
	}
}	
static void printClientsOfStateStream(void){
	
	for(u_int64_t i=0;i<acessVarMtx(&varMtx,&state->clientsActive,0,-1);i++){
		
		clientStruct* currClient=(clientStruct*)acessListMtx(&listMtx,state->listOfClients,0,i,2);
		
		printw("Client number %lu: %s\nLogin: %s\nLiga-nos da porta: %d\nTem numero de socket: %d\nJa recebeu %lu bytes:\nJa foi despachado???????%lu\n",i,inet_ntoa(currClient->clientAddress.sin_addr),currClient->login,(int) ntohs(currClient->clientAddress.sin_port),(int32_t)acessVarMtx(&varMtx,&currClient->client_socket,0,-1),acessVarMtx(&varMtx,&currClient->numOfBytesSent,0,-1),acessVarMtx(&varMtx,&currClient->done,0,-1));
	}
}	
static void printAdminsOfStateStream(void){
	
	for(u_int64_t i=0;i<acessVarMtx(&varMtx,&state->adminsActive,0,-1);i++){
		
		clientStruct* currClient=(clientStruct*)acessListMtx(&listMtx,state->listOfAdmins,0,i,2);
		printw("Admin number %lu: %s.\nLiga-nos da porta: %d\nTem numero de socket: %d\n\n",i,inet_ntoa(currClient->clientAddress.sin_addr),(int) ntohs(currClient->clientAddress.sin_port),(int32_t)acessVarMtx(&varMtx,&currClient->client_socket,0,-1));
	}	
}	

void printServerState(void){
	
	
	char date[LOGMSGLENGTH]={0};
	formatCurrTime(date);
	printw("Endereco do server: %s\n",inet_ntoa(state->server_address.sin_addr));
	printw("Data: %s\nDatasize %lu\nPingsize %u\nNumero de clientes: %lu (de %lu (Hard limit:%lu))\nNumero de admins: %lu\nServidor ativo? %lu\nServidor a hibernar? %lu\nNumero de bytes enviados no total: %lu\nTaxa de trafego: %lf\nClientes: \n",date,acessVarMtx(&varMtx,&state->dataSize,0,-1),(u_int16_t)acessVarMtx(&varMtx,&state->pingSize,0,-1),acessVarMtx(&varMtx,&state->clientsActive,0,-1),acessVarMtx(&varMtx,&state->maxNumOfClients,0,-1),MAX_CLIENTS_HARD_LIMIT,acessVarMtx(&varMtx,&state->adminsActive,0,-1),acessVarMtx(&varMtx,&state->serverRunning,0,-1),acessVarMtx(&varMtx,&state->idle,0,-1),acessVarMtx(&varMtx,&state->totalSent,0,-1),(double)acessVarMtx(&varMtx,(u_int64_t*)&state->trafficRate,0,-1));
	printClientsOfStateStream();
	printAdminsOfStateStream();
	refresh();
	erase();
}

void printServerLogs(void){

	while(*(u_int64_t*)acessStackMtx(&stackMtx,state->logs,0,3)){
		char* buff=acessStackMtx(&stackMtx,state->logs,NULL,1);
		write(logfd,buff,min(LOGMSGLENGTH,strlen(buff)));
		write(logfd,"\n",1);
		free(buff);
	}
	
	usleep(250000);
	
}



