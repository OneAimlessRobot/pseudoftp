#include "../Includes/preprocessor.h"
#include "../Includes/login.h"
#include "../Includes/accounts.h"


extern hashtablecomp* loadedLogins;
extern int loginfd;
extern char* loginsFile;
void loadLogins(){
	
	loadedLogins=initHashTableComp(sizeof(loginStruct),&loginComparator,&loginHasher);

	
	loginfd=open(loginsFile,O_RDWR,0777);
	if(loginfd<0){

		perror("Could not load logins file!!!!!!!\n");
		exit(-1);
	}
	FILE* stream= fdopen(loginfd,"r");
	char* buff=malloc(LINESIZE);
	u_int64_t *charsToRead=malloc(sizeof(u_int64_t));
	*charsToRead=LINESIZE;
	while(getdelim(&buff,charsToRead,'\n',stream)>0){
		
		loginStruct newLogin;
		memset(newLogin.user,0,FIELDLENGTH+1);
		memset(newLogin.password,0,FIELDLENGTH+1);
		sscanf(buff,"%s %s",newLogin.user,newLogin.password);
		addToHTComp(&loadedLogins,&newLogin);
		
	}
	free(buff);
	free(charsToRead);
	fclose(stream);



}
void saveLogins(){
	
	
	htiterator* it=initItHT(loadedLogins);
	
	FILE* stream= fopen(loginsFile,"w");
	if(stream<0){

		perror("Nao foi possivel salvar logins!!!\n");
		return;
	}
	while(hasNextItHT(it)){
		
		loginStruct* next= (loginStruct*)nextItHT(it);
		
		fprintf(stream,"%s %s\n",next->user,next->password);
		
	}
	fclose(stream);
	if(it->currIt){

		free(it->currIt);
		it->currIt=NULL;
	}
	free(it);



}


