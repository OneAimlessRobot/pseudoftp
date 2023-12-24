

#include "Includes/preprocessor.h"

int fd;
extern serverState* state;
int main(int argc, char ** argv){
	if(argc!=6){

		printf("Precisas de uma porta para o server!!!\nE de um ficheiro para copiar\nE de um tamanho de ping (em bytes)\nE um tamanho de dados (em bytes)\nE do numero maximo de clients\n");
		exit(-1);
	}
	
	u_int16_t port= (u_int16_t)atoi(argv[1]);
	initEverything(port,argv[2],atoi(argv[3]),atoi(argv[4]),atoi(argv[5]));
	//bind the socket to the specified IP and port
	
	cleanup();
	return 0;
	
}


