#ifndef MSG_H
#define MSG_H
#define CMDLENGTH 10
typedef struct msg{

	char cmd[CMDLENGTH];
	char* data;
	u_int64_t dataLength;

}msg;


msg* makeMsg(char cmd[CMDLENGTH], char* data, u_int64_t dataLength);

char* parseMsg(msg* msg);


void destroyMsg(msg** msg);


#endif
