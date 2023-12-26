#ifndef CLIENT_H
#define CLIENT_H

int notifyClientAboutSizes(clientStruct* currClient,int numRead);

int receiveWholeClientPing(clientStruct*client,char message[],u_int64_t size);

#endif
