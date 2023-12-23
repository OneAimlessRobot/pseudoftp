#include "../Includes/preprocessor.h"
#include "../Includes/comparator.h"
#include "../Includes/hasher.h"
#include "../Includes/login.h"



hasher loginHasher= {hashLogin};
comparator loginComparator= {compareLogins};


int compareLogins(void* cmd1,void*cmd2){

	loginStruct* var1= (loginStruct*)cmd1;
	loginStruct* var2= (loginStruct*)cmd2;

	return strcmp(var1->user,var2->user);
	

}



int64_t hashLogin(char* login){


char* mem= ((loginStruct*)(login))->user;

u_int64_t hash= 5381;

	u_int64_t count=0;

for(;count<FIELDLENGTH+1;count++){

hash= ((hash << 5) + hash) +mem[count];
}
return hash;



}

loginStruct* spawnLoginStruct(char* user, char* password){

	loginStruct* result= malloc(sizeof(loginStruct));
	memset(result->user,0,FIELDLENGTH+1);
	memset(result->password,0,FIELDLENGTH+1);
	memcpy(result->user,user,min(FIELDLENGTH,strlen(user)));
	memcpy(result->password,password,min(FIELDLENGTH,strlen(password)));
	return result;


}
void destroyLogin(loginStruct* login){

	free(login);
}


