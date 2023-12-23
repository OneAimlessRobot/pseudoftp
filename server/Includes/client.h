#ifndef CLIENT_H
#define CLIENT_H

int receiveClientPing(clientStruct* nextClient,char buff[],u_int64_t size);

int receiveClientField(clientStruct* nextClient,char buff[],u_int64_t size);

int notifyClientAboutSizes(clientStruct* currClient);

#endif
