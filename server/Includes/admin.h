#ifndef ADMIN_H
#define ADMIN_H
int receiveClientCommand(clientStruct* nextClient,char buff[LINESIZE]);

void enqueueCommand(char cmdbuff[LINESIZE],clientStruct* nextClient);


#endif
