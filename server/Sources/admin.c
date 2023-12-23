#include "../Includes/preprocessor.h"
#include "../Includes/admin.h"
#include "../Includes/infometers.h"

extern pthread_mutex_t varMtx;
extern serverState* state;

int receiveClientCommand(clientStruct* nextClient,char buff[LINESIZE]){
		int iResult;
		struct timeval tv;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET((int)acessVarMtx(&varMtx,&nextClient->client_socket,0,-1),&rfds);
		tv.tv_sec=MAXTIMEOUTPING;
		tv.tv_usec=MAXTIMEOUTPINGU;
		iResult=select((int)acessVarMtx(&varMtx,&nextClient->client_socket,0,-1)+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
		if(iResult>0){
		return recv((int)acessVarMtx(&varMtx,&nextClient->client_socket,0,-1),buff,LINESIZE,0);
		
		}
		return -1;

}

void enqueueCommand(char cmdbuff[LINESIZE],clientStruct* nextClient){


			command cmd;
			memset(cmd.string,0,LINESIZE);
			strncpy(cmd.string,cmdbuff,LINESIZE);
			char buff[LOGMSGLENGTH];
			snprintf(buff,LOGMSGLENGTH,"Comando executado!!!! %s\n",cmd.string);
			pushLog(buff);
			//printf("Comando executado!!!! %s\n",cmd.string);
			cmd.whoItWas=nextClient;
	
}
