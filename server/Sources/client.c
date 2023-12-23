#include "../Includes/preprocessor.h"
#include "../Includes/client.h"
#include "../Includes/infometers.h"

extern const char* pingCorrect;

extern serverState* state;
extern pthread_mutex_t varMtx;

int receiveClientPing(clientStruct* nextClient,char buff[],u_int64_t size){
		u_int16_t pingSize=(u_int16_t)acessVarMtx(&varMtx,&state->dataSize,0,-1);
		int client_socket=(int)acessVarMtx(&varMtx,&nextClient->client_socket,0,-1);
		int iResult;
		struct timeval tv;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(client_socket,&rfds);
		tv.tv_sec=MAXTIMEOUTPING;
		tv.tv_usec=MAXTIMEOUTPINGU;
		iResult=select(client_socket+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
		if(iResult>0){
		return recv(client_socket,buff,size,0);
		
		}

}
int receiveClientField(clientStruct* nextClient,char buff[],u_int64_t size){
		int client_socket=(int)acessVarMtx(&varMtx,&nextClient->client_socket,0,-1);
		return recv(client_socket,buff,size,0);
}


int notifyClientAboutSizes(clientStruct* currClient){
u_int16_t pingSize=(u_int16_t)acessVarMtx(&varMtx,&state->pingSize,0,-1);
u_int64_t dataSize=acessVarMtx(&varMtx,&state->dataSize,0,-1);
char ping[pingSize];
memset(ping,0,pingSize);
int client_socket=(int)acessVarMtx(&varMtx,&currClient->client_socket,0,-1);
int fd=(int)acessVarMtx(&varMtx,&currClient->fd,0,-1);
		snprintf(ping,pingSize,"%u %lu",pingSize, dataSize);
		send(client_socket,ping,pingSize,0);
		
		int status=receiveClientPing(currClient,ping,pingSize);
		//printf("%d %hu\n",status,pingSize);
		if(status>0){
			if(!strncmp(ping,pingCorrect,strlen(pingCorrect))){
			char buff3[LOGMSGLENGTH]={0};
			snprintf(buff3,LOGMSGLENGTH,"client got the sizes!!!!!!!");
			pushLog(buff3);
			
			//printf("client got the sizes!!!!!!!\n");
			}
		}
		else {
			
			char buff3[LOGMSGLENGTH]={0};
			snprintf(buff3,LOGMSGLENGTH,"Timed out. Dropping...");
			pushLog(buff3);
			
			//printf("Timed out. Dropping...\n");
		}
		return status;

}
