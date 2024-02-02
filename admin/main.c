#include "Includes/preprocessor.h"

struct sockaddr_in server_address;
static u_int64_t dataSize;

const char* pingCorrect="queroja";

#define MAXNUMBEROFTRIES 10
#define MAXTIMEOUTSECS 1
#define LINESIZE 1024
#define MAXTIMEOUTUSECS 0
#define MAXTIMEOUTCONS 3
#define MAXTIMEOUTUCONS 0
#define PINGSIZE 50
#define FIELDLENGTH 127
int client_socket;
int fd;

void tryConnect(int* socket){
        int success=-1;
        int numOfTries=MAXNUMBEROFTRIES;


        while(success==-1&& numOfTries){
                printf("Tentando conectar a %s (Tentativa %d)!!!!!!\n",inet_ntoa(server_address.sin_addr),-numOfTries+MAXNUMBEROFTRIES+1);
                success=connect(*socket,(struct sockaddr*)&server_address,sizeof(server_address));
               int sockerr;
                socklen_t slen=sizeof(sockerr);
                getsockopt(*socket,SOL_SOCKET,SO_ERROR,(char*)&sockerr,&slen);
                numOfTries--;
                if(sockerr==EINPROGRESS){

                        fprintf(stderr,"Erro normal:%s\n Erro Socket%s\nNumero socket: %d\n",strerror(errno),strerror(sockerr),*socket);
                        continue;

                }
                fd_set wfds;
                FD_ZERO(&wfds);
                FD_SET((*socket),&wfds);

                struct timeval t;
                t.tv_sec=MAXTIMEOUTCONS;
                t.tv_usec=MAXTIMEOUTUCONS;
                int iResult=select((*socket)+1,0,&wfds,0,&t);

                if(iResult>0&&!success&&((*socket)!=-1)){
                //printf("Coneccao de %s!!!!!!\n",inet_ntoa(server_address.sin_addr));
                        break;

                }
                fprintf(stderr,"Não foi possivel: %s\n",strerror(errno));
        }
        if(!numOfTries){
        printf("Não foi possivel conectar. Numero limite de tentativas (%d) atingido!!!\n",MAXNUMBEROFTRIES);
        raise(SIGINT);
        }

}

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

	if(argc!=3){

		printf("Utilizacao correta: arg1: porta do server.\narg2: ipv4 do server\n");
		exit(-1);
	}
	//especificar socket;
	client_socket= socket(AF_INET,SOCK_STREAM,0);
	if(client_socket==-1){
		raise(SIGINT);
	}

       long flags= fcntl(client_socket,F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(client_socket,F_SETFD,flags);


	signal(SIGINT,sigint_handler);
	server_address.sin_family=AF_INET;
	server_address.sin_port= htons(atoi(argv[1]));
	server_address.sin_addr.s_addr = inet_addr(argv[2]);	
	int success=-1;
	int numOfTries=MAXNUMBEROFTRIES;
	tryConnect(&client_socket);
	printf("Conectado a %s!!!!!!\n",inet_ntoa(server_address.sin_addr));
	
	//receber e armazenar dados recebidos
        

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

