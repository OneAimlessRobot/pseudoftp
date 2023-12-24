#include "../Includes/preprocessor.h"
#include "../Includes/msg.h"


msg* makeMsg(char cmd[CMDLENGTH], char* data, u_int64_t dataLength){
	msg* result= malloc(sizeof(msg));
	memset(result->cmd,0,CMDLENGTH);
	memcpy(result->cmd,cmd,CMDLENGTH);
	
	result->data= malloc(dataLength);
	memset(result->data,0,dataLength);
	
	memcpy(result->data,data,dataLength);
	result->dataLength=dataLength;

	return result;
}

char* parseMsg(msg* msg){
	if(msg){
	char number[1024];
	memset(number,0,1024);
	snprintf(number,1024,"%d",msg->dataLength);
	u_int64_t msgSize=msg->dataLength+1+strlen(number)+1+CMDLENGTH+1;
	char* result= malloc(msgSize);
	snprintf(result,msgSize,"%s %d %s",msg->cmd,msg->dataLength,msg->data);
	return result;
			
	}
	return NULL;
}


void destroyMsg(msg** msg){

	free((*msg)->data);
	(*msg)->data=NULL;
	free((*msg));
	*msg=NULL;

}

