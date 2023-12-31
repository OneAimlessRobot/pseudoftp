#include "Includes/preprocessor.h"

static u_int64_t dataSize;
const char* pingCorrect="queroja";
#define MAXNUMBEROFTRIES 10
#define MAXTIMEOUTSECS 3
#define MAXTIMEOUTUSECS 0
#define FIELDLENGTH 127
#define PINGSIZE 50
int client_socket;
int fd;

static int receiveServerPing(char buff[],u_int64_t size){
                int iResult;
                struct timeval tv;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(client_socket,&rfds);
                tv.tv_sec=MAXTIMEOUTSECS;
                tv.tv_usec=MAXTIMEOUTUSECS;
                iResult=select(client_socket+1,&rfds,(fd_set*)0,(fd_set*)0,&tv);
                if(iResult>0){
		
		return recv(client_socket,buff,size,0);
		
                }
		return -1;
}


static int receiveWholeServerPing(char message[],u_int64_t size){
		int counter=0;
	int64_t len=0;
	int64_t total=0;

for (; total<size;) { /* Watch out for buffer overflow */
     	total+=len=receiveServerPing(message+total,size-total);
	if(len<0){
		break;
	}
}
	return total;

}
void loginScreen(){
	char userPrompt[FIELDLENGTH+1];
        char buff3[FIELDLENGTH +1];
	
	memset(userPrompt,0,FIELDLENGTH+1);
	receiveWholeServerPing(userPrompt,FIELDLENGTH+1);
	printf("%s",userPrompt);
	fflush(stdout);
	memset(buff3,0,FIELDLENGTH +1);
	scanf("%s",buff3);
        send(client_socket,buff3,FIELDLENGTH +1,0);
	
	memset(userPrompt,0,FIELDLENGTH+1);
	receiveWholeServerPing(userPrompt,FIELDLENGTH+1);
	printf("%s",userPrompt);
	fflush(stdout);
	memset(buff3,0,FIELDLENGTH +1);
	scanf("%s",buff3);
        send(client_socket,buff3,FIELDLENGTH +1,0);
	


}
void sigpipe_handler(int signal){
/*
	close(client_socket);
	close(fd);*/
	printf("%s\n",strerror(errno));
//	exit(-1);
	//exit(-1);
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
	signal(SIGPIPE,sigint_handler);
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
	char buff[PINGSIZE]={0};
	
        receiveWholeServerPing(buff,PINGSIZE);
	send(client_socket,buff,PINGSIZE,0);
        sscanf(buff,"%lu",&dataSize);
	printf("Tamanhos:\ndados: %lu\n",dataSize);
	loginScreen();
	memset(buff,0,PINGSIZE);
	receiveWholeServerPing(buff,PINGSIZE);
        printf("%s\n",buff);
	//R S
	while(1){
		char buff[PINGSIZE];
		char pingBuff[PINGSIZE];
		memset(buff,0,PINGSIZE);
		memset(pingBuff,0,PINGSIZE);
		int status=receiveWholeServerPing(buff,PINGSIZE);
		if(status<0){
			printf("O client vai sair porque n recebeu um ping inteiro!!!!\n");
			raise(SIGINT);
		}
		sscanf(buff,"%lu",&dataSize);
		send(client_socket,pingCorrect,strlen(pingCorrect),0);

		printf("Tamanhos:\ndados: %lu\n",dataSize);
	
	
		char message[dataSize];
		memset(message,0,dataSize);
	int total= receiveWholeServerPing(message,dataSize);
		if(total<0){
			printf("O client vai sair porque n recebeu um chunk inteiro!!!!\n");
			raise(SIGINT);
		}
		//system("clear");
		status=write(fd,message,total);
		printf("Received chunk of data with size %d from %s\n\nWritting....to here:%s\n",status,inet_ntoa(server_address.sin_addr),argv[1]);

//		printf("Recebidos: %d \n",counter);
		if(!status){
			printf("No bytes were written!!! End of file...\n");
		}
		else if(status==-1){
			perror("No bytes written!!!! An error happened\n");
		}
		printf("Done!!!!!\n");
		snprintf(pingBuff,PINGSIZE,"%d",status);
		send(client_socket,pingBuff,PINGSIZE,0);
		
	}

	raise(SIGINT);
	return 0;
}

