#ifndef HASHTABLECMD_H
#define HASHTABLECMD_H

hashtablecomp* initCmdLine(cmdstruct commands[]);

void runCmdLine(hashtablecomp* cmdLookupTable,clientStruct* client,char line[LINESIZE],char outputbuff[LINESIZE]);

#endif
