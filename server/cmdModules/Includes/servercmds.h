#ifndef SERVERCMDS_H
#define SERVERCMDS_H

extern cmdstruct servercmds[];

void showservercmds(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getTotalDataTransfered(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getPingSize(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getDataSize(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getAdminQuota(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getClientQuota(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getMaxClientQuota(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getClientList(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getAdminList(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void setMaxClientQuota(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void setPingSize(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void setDataSize(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void sair(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getLoadedLogins(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void killServer(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void getMaxClientHardLimit(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);

void addLogin(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]);


#endif
