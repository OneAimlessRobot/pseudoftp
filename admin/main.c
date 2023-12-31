#include "Includes/preprocessor.h"

static u_int64_t dataSize;

const char* pingCorrect="queroja";

#define MAXNUMBEROFTRIES 10
#define MAXTIMEOUTSECS 1
#define LINESIZE 1024
#define MAXTIMEOUTUSECS 0
#define PINGSIZE 50
#define FIELDLENGTH 127
int client_socket;
int fd;

static int64_t receiveServerOutput(char buff[],u_int64_t size){
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

static int receiveWholeServerOutput(char message[],u_int64_t size){
                int counter=0;
        int64_t len=0;
        int64_t total=0;

for (; total<size;) { /* Watch out for buffer overflow */
        total+=len=receiveServerOutput(message+total,size-total);

}
        return total;

}

void loginScreen(){
 
 	char userPrompt[FIELDLENGTH+1];
        char buff3[FIELDLENGTH +1];

        memset(userPrompt,0,FIELDLENGTH+1);
        receiveWholeServerOutput(userPrompt,FIELDLENGTH+1);
        printf("%s",userPrompt);
        fflush(stdout);
        memset(buff3,0,FIELDLENGTH +1);
        scanf("%s",buff3);
        send(client_socket,buff3,FIELDLENGTH +1,0);

        memset(userPrompt,0,FIELDLENGTH+1);
        receiveWholeServerOutput(userPrompt,FIELDLENGTH+1);
        printf("%s",userPrompt);
        fflush(stdout);
        memset(buff3,0,FIELDLENGTH +1);
        scanf("%s",buff3);
        send(client_socket,buff3,FIELDLENGTH +1,0);




}

void sigint_handler(int signal){

	close(client_socket);
	close(fd);
	printf("cliente a fechar!!!\n");
	exit(-1);

}
int main(int argc, char ** argv){

	if(argc!=4){

		printf("Utilizacao correta: arg1: Ficheiro de destino.  arg2: porta do server.\narg3: ipv4 do server\n");
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
	}
	signal(SIGINT,sigint_handler);
	struct sockaddr_in server_address;
	server_address.sin_family=AF_INET;
	server_address.sin_port= htons(atoi(argv[2]));
	server_address.sin_addr.s_addr = inet_addr(argv[3]);	
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
        
	char buff[PINGSIZE];
	receiveWholeServerOutput(buff,PINGSIZE);
        sscanf(buff,"%lu",&dataSize);

        printf("Tamanhos:\ndados: %lu\n",dataSize);
	send(client_socket,buff,PINGSIZE,0);

	loginScreen();	
	char buff3[1024]={0};
	receiveWholeServerOutput(buff3,1024);
	printf("%s\n",buff3);
	fflush(stdout);
	while(1){
		char buff[LINESIZE];
		memset(buff,0,LINESIZE);
		

		printf("admin>: ");
		fflush(stdout);
		fgets(buff,LINESIZE,stdin);
		send(client_socket,buff,LINESIZE,0);
		
                int status=receiveWholeServerOutput(buff,LINESIZE);
                if(status<0||!strncmp(buff,"die",LINESIZE)){
                        raise(SIGINT);
                }
		
		printf("%s\n",buff);
	}

	raise(SIGINT);
}

