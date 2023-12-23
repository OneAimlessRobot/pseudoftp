#include "Includes/preprocessor.h"

int main(int argc, char ** argv){

	int network_socket;

	network_socket= socket(AF_INET,SOCK_STREAM,0);

	//especificar socket;

	struct sockaddr_in server_address;
	server_address.sin_family=AF_INET;
	server_address.sin_port= htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;
	
	int success=connect(network_socket,(struct sockaddr*)&server_address,sizeof(server_address));

	if(success<0){

		printf("TUDO MAL erro na conexão!!!!!\n");
	}

	//receber e armazenar dados recebidos
	char infogotten[10000];
	memset(infogotten,0,10000);
	recv(network_socket,&server_address,sizeof(infogotten),0);
	
	//imprimir dados recebidos
	printf("Recebemos: %s\n2", infogotten);
	close(network_socket);
	return 0;
}
