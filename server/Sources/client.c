#include "../Includes/preprocessor.h"
#include "../Includes/client.h"
#include "../Includes/infometers.h"

extern const char* pingCorrect;
extern serverState* state;
extern pthread_mutex_t varMtx;

static int receiveClientPing(clientStruct* nextClient,char buff[],u_int64_t size){
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
int receiveWholeClientPing(clientStruct*client,char message[],u_int64_t size){
                int counter=0;
        int64_t len=0;
        int64_t total=0;

for (; total<size;) { /* Watch out for buffer overflow */
        total+=len=receiveClientPing(client,message+total,size-total);
	if(len<0){
                break;
        }

}
        return total;

}

int notifyClientAboutSizes(clientStruct* currClient,int numRead){
u_int64_t dataSize=acessVarMtx(&varMtx,&state->dataSize,0,-1);
char ping[PINGSIZE];
memset(ping,0,PINGSIZE);
int client_socket=(int)acessVarMtx(&varMtx,&currClient->client_socket,0,-1);
int fd=(int)acessVarMtx(&varMtx,&currClient->fd,0,-1);
		snprintf(ping,PINGSIZE,"%d", numRead);
		send(client_socket,ping,PINGSIZE,0);
		
		int status=receiveWholeClientPing(currClient,ping,PINGSIZE);
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
