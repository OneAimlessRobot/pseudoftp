#include "Includes/preprocessor.h"

static u_int16_t pingSize;
static u_int64_t dataSize;
const char* pingCorrect="queroja";
#define MAXNUMBEROFTRIES 10
#define MAXTIMEOUTSECS 100
#define MAXTIMEOUTUSECS 0
int client_socket;
int fd;

static int receiveServerPing(char buff[],u_int64_t size){
                int iResult;
                struct timeval tv;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(client_socket,&rfds);
                tv.tv_sec=MAXTIMEOUTSECS;
                tv.tv_usec=0;
                iResult=select(client_socket+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
		
		return recv(client_socket,buff,size,0);
		
                }
		return -1;
}

void loginScreen(){
	char userPrompt[pingSize];
        char buff3[pingSize];
	
	memset(userPrompt,0,pingSize);
	receiveServerPing(userPrompt,pingSize);
	printf("%s",userPrompt);
	fflush(stdout);
	memset(buff3,0,pingSize);
	scanf("%s",buff3);
        send(client_socket,buff3,pingSize,0);
	
	memset(userPrompt,0,pingSize);
	receiveServerPing(userPrompt,pingSize);
	printf("%s",userPrompt);
	fflush(stdout);
	memset(buff3,0,pingSize);
	scanf("%s",buff3);
        send(client_socket,buff3,pingSize,0);
	


}
void sigint_handler(int signal){

	close(client_socket);
	close(fd);
	printf("cliente a fechar!!!\n");
	exit(-1);

}
int main(int argc, char ** argv){

	if(argc!=4){

		printf("Utilizacao correta: arg1: Ficheiro de destino. \n arg2: porta do server.\narg3: ipv4 do server\n");
		exit(-1);
	}
	//especificar socket;
	fd= creat(argv[1],0777);
	if(fd<0){
		perror("Não foi possivel criar destino dos dados recebidos.\n");
		exit(-1);
	}
	client_socket= socket(AF_INET,SOCK_STREAM,0);
	if(client_socket==-1){
		raise(SIGINT);
		return 0;
	}
	signal(SIGINT,sigint_handler);
	struct sockaddr_in server_address;
	server_address.sin_family=AF_INET;
	server_address.sin_port= htons(atoi(argv[2]));
	struct hostent* hp= gethostbyname(argv[3]);
	//server_address.sin_addr.s_addr = inet_addr(argv[3]);	
	memcpy(&(server_address.sin_addr),hp->h_addr,hp->h_length);
	int success=-1;
	int numOfTries=MAXNUMBEROFTRIES;
	while(success==-1&& numOfTries){
		printf("Tentando conectar a %s: Tentativa %d\n",inet_ntoa(server_address.sin_addr),-numOfTries+MAXNUMBEROFTRIES+1);
		usleep(1000000);
		success=connect(client_socket,(struct sockaddr*)&server_address,sizeof(server_address));
		numOfTries--;
		if(success>=0){
		break;	
		}
		fprintf(stderr,"Não foi possivel: %s\n",strerror(errno));
	}
	if(!numOfTries){
	printf("Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAXNUMBEROFTRIES);
	raise(SIGINT);
	}
	printf("Conectado a %s!!!!!!\n",inet_ntoa(server_address.sin_addr));
	
	//receber e armazenar dados recebidos
	char buff[1024];
	memset(buff,0,1024);
	
        recv(client_socket,buff,1024,0);
	send(client_socket,buff,pingSize,0);
	char buff2[1024]={0};
        sscanf(buff,"%hu %lu %s",&pingSize,&dataSize,buff2);
	printf("Tamanhos:\npings: %hu\ndados: %lu\n",pingSize,dataSize);
	loginScreen();
	memset(buff,0,pingSize);
        recv(client_socket,buff,pingSize,0);
        printf("%s\n",buff);
	//R S
	while(1){
		char buff[pingSize];
		memset(buff,0,pingSize);
		int64_t status=receiveServerPing(buff,pingSize);
		if(status<0){
			raise(SIGINT);
		}
		sscanf(buff,"%hu %lu",&pingSize,&dataSize);
		send(client_socket,pingCorrect,strlen(pingCorrect),0);

		//printf("Tamanhos:\npings: %hu\ndados: %lu\n",pingSize,dataSize);
	
	
		

		char message[dataSize];
		memset(message,0,dataSize);
		status=receiveServerPing(message,dataSize);
		if(status<=0){
			raise(SIGINT);
		}
		printf("Received chunk of data with size %ld from %s\n\nWritting....to here:%s\n",status,inet_ntoa(server_address.sin_addr),argv[1]);
		status=write(fd,message,status);
		if(!status){
			printf("No bytes were written!!! End of file...\n");
		}
		else if(status==-1){
			perror("No bytes written!!!! An error happened\n");
		}
		printf("Done!!!!!\n");
		send(client_socket,pingCorrect,strlen(pingCorrect),0);

		
	}

	raise(SIGINT);
	return 0;
}

