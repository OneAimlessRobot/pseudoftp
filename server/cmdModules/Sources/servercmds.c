#include "../../Includes/preprocessor.h"
#include "../../Includes/cmdstruct.h"
#include "../Includes/servercmds.h"
#include "../../Includes/atomicOps.h"
#include "../../Includes/hud.h"
#include "../../Includes/login.h"
#include "../../Includes/accounts.h"
cmdstruct servercmds[]={
			{"sps",setPingSize,"Altera tamanho de ping"},
			{"sds",setDataSize,"Altera tamanho de dados"},
			{"scq",setMaxClientQuota,"Muda o numero maximo de clients (Nao deve ser maior que o hard limit dito por 'chl')"},
			{"alogin",addLogin,"Adiciona uma nova conta (username e password)"},
			{"gps",getPingSize,"Consulta tamanho de ping"},
			{"gds",getDataSize,"Consulta tamanho de dados"},
			{"gaq",getAdminQuota,"Consulta numero de admins"},
			{"gcq",getClientQuota,"Consulta numero de clients"},
			{"gcl",getClientList,"Consulta a lista de clients"},
			{"gll",getLoadedLogins,"Consulta a lista de logins carregados"},
			{"gal",getAdminList,"Consulta a lista de admins"},
			{"gmcq",getMaxClientQuota,"Consulta numero de clients maximo"},
			{"gtdt",getTotalDataTransfered,"Consulta a quantidade de dados ja transferida"},
			{"chl",getMaxClientHardLimit,"Consulta o hard limit para scq (definido quando o server e compilado)"},
			{"e",sair,"sair deste pesadelo"},
			{"kill",killServer,"acabar com o pesadelo"},
			{"sh",showservercmds,"ver comandos"},
			{"",0,""}
		};

extern serverState* state;
extern hashtablecomp * loadedLogins;
extern pthread_mutex_t varMtx;
extern pthread_mutex_t stackMtx;
extern pthread_cond_t kickingCond;
extern pthread_mutex_t listMtx;

void setPingSize(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	if(argc!=2){

		snprintf(buff,LINESIZE,"Numero errado de argumentos!\n");
		return;
	}
	
	acessVarMtx(&varMtx,&state->pingSize,atoi(argv[1]),0);

}

void setDataSize(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){


	if(argc!=2){

		snprintf(buff,LINESIZE,"Numero errado de argumentos!\n");
		return;
	}
	acessVarMtx(&varMtx,&state->dataSize,atoi(argv[1]),0);

}

void getPingSize(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	snprintf(buff,LINESIZE,"Tamanho de ping: %hu bytes.\n",(u_int16_t)acessVarMtx(&varMtx,&state->pingSize,0,-1));
          
}

void getDataSize(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	snprintf(buff,LINESIZE,"Tamanho de dados: %lu bytes.\n",acessVarMtx(&varMtx,&state->dataSize,0,-1));
}
void getAdminQuota(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	snprintf(buff,LINESIZE,"Estão %lu admins no server.\n",*(u_int64_t*)acessListMtx(&listMtx,state->listOfAdmins,0,0,4));
          
}
void getClientQuota(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	snprintf(buff,LINESIZE,"Estão %lu clientes no server.\n",*(u_int64_t*)acessListMtx(&listMtx,state->listOfClients,0,0,4));
          
}
void getTotalDataTransfered(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	snprintf(buff,LINESIZE,"Ja foram transferidos %lu bytes.\n",acessVarMtx(&varMtx,&state->totalSent,0,-1));
          
}
void getMaxClientQuota(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	snprintf(buff,LINESIZE,"São permitidos ate %lu clientes no server. (nao deve ser aumentado acima do valor inicial)\n",acessVarMtx(&varMtx,&state->maxNumOfClients,0,-1));
          
}
void setMaxClientQuota(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){
	if(argc!=2){

		snprintf(buff,LINESIZE,"Numero errado de argumentos!\n");
		return;
	}
	u_int64_t finalResult= min(atoi(argv[1]),MAX_CLIENTS_HARD_LIMIT);
	acessVarMtx(&varMtx,&state->maxNumOfClients,abs(finalResult),0);
          
}

void sair(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	snprintf(buff,LINESIZE,"die");
         acessStackMtx(&stackMtx,state->kickedClients,client,0);
	pthread_cond_signal(&kickingCond);


}


void showservercmds(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){


                        
			int j=0;
                        char* curr= buff;
			while(servercmds[j].cmdname[0]){
                        curr+=snprintf(curr,LINESIZE,"%s- %s\n",(char*)servercmds[j].cmdname,(char*)servercmds[j].helpdesc);
                        j++;

                        }
                        curr+=snprintf(curr,LINESIZE,"\n");
}

void killServer(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	
	raise(SIGINT);

}

void getClientList(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){
	printClientsOfState(buff);
}
void getAdminList(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	printAdminsOfState(buff);

}

void getMaxClientHardLimit(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){

	snprintf(buff,LINESIZE,"%lu",MAX_CLIENTS_HARD_LIMIT);

}


void getLoadedLogins(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){
        htiterator* it=initItHT(loadedLogins);
	char* curr=buff;
        while(hasNextItHT(it)){

                loginStruct* next= (loginStruct*)nextItHT(it);

                curr+=snprintf(curr,2*FIELDLENGTH+4,"%s %s\n",next->user,next->password,strlen(next->user));

        }
        if(it->currIt){

                free(it->currIt);
                it->currIt=NULL;
        }
        free(it);



}

void addLogin(int64_t argc,clientStruct*client, void** argv,char buff[LINESIZE]){
        
	if(argc!=3){

	snprintf(buff,LINESIZE,"Username e password necessarios!!!!\n");
	return;

	}
	loginStruct* added= spawnLoginStruct(argv[1],argv[2]);
	addToHTComp(&loadedLogins,added);
	snprintf(buff,LINESIZE,"Username %s adicionado!!!!!!!!\n",added->user);
	free(added);

}

