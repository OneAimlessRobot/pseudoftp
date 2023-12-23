#include  "../Includes/preprocessor.h"
#include "../Includes/hashtablecomp.h"
#include "../Includes/hashtablecmd.h"
#include "../Includes/auxFuncs.h"
#include "../Includes/cmdstruct.h"

hashtablecomp* initCmdLine(cmdstruct commands[]){


	if(!commands){

		return NULL;
	}
	hashtablecomp* result= initHashTableComp(sizeof(cmdstruct),&cmdcomparator,&cmdhasher);
	int i=0;
	while(commands[i].cmdname[0]){
		cmdstruct* newcmd= spawnCmdStruct(commands[i].cmdname,commands[i].cmd,commands[i].helpdesc);
		addToHTComp(&result,newcmd);
		free(newcmd);
		i++;
	}

	return result;


}
static int makeargv(char *s, char *argv[ARGVMAX]) {
    int ntokens = 0;

    if (s == NULL || argv == NULL || ARGVMAX == 0)
        return -1;
    argv[ntokens] = strtok(s, " \t\n");
    while ((argv[ntokens] != NULL) && (ntokens < ARGVMAX)) {
        ntokens++;
        argv[ntokens] = strtok(NULL, " \t\n");
    }
    argv[ntokens] = NULL; // it must terminate with NULL
    return ntokens;
}

void runCmdLine(hashtablecomp* cmdLookupTable,clientStruct* client,char line[LINESIZE],char outputbuff[LINESIZE]){

        char* argv[ARGVMAX];
      memset(argv,0,ARGVMAX*sizeof(char*));

                line[strlen(line)-1]=0;
                int64_t numOfArgs=  makeargv(line, argv);
                if(argv[0]){
                cmdstruct* cmd =spawnCmdStruct(argv[0],NULL,"");
                cmdstruct* result= getHTElemComp(cmdLookupTable,(void*)cmd);
                free(cmd);
		//printf("Comando:%s\n",line);
                if(!result){
                        return;
                }

                result->cmd(numOfArgs,client,(void**)argv,outputbuff);
                }

}

