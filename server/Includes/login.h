#ifndef LOGIN_H
#define LOGIN_H
typedef struct loginStruct{

	char user[FIELDLENGTH+1];
	char password[FIELDLENGTH+1];

}loginStruct;


int compareLogins(void* cmd1,void*cmd2);

int64_t hashLogin(char* cmd);

loginStruct* spawnLoginStruct(char* user,char* password);

void destroyLogin(loginStruct* login);

extern hasher loginHasher;
extern comparator loginComparator;

#endif
