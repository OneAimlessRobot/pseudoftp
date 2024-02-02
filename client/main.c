#include "Includes/preprocessor.h"
struct sockaddr_in server_address;
static u_int64_t dataSize;
const char* pingCorrect="queroja";
#define MAXNUMBEROFTRIES 10
#define MAXTIMEOUTSECS 3
#define MAXTIMEOUTUSECS 0
#define MAXTIMEOUTCONS 3
#define MAXTIMEOUTUCONS 0
#define FIELDLENGTH 127
#define PINGSIZE 50
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
		raise(SIGINT);
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

	close(client_socket);
	close(fd);
	printf("%s\n",strerror(errno));
	exit(-1);
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
	/*
       long flags= fcntl(client_socket,F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(client_socket,F_SETFD,flags);
	*/
	signal(SIGINT,sigint_handler);
	signal(SIGPIPE,sigint_handler);
	server_address.sin_family=AF_INET;
	server_address.sin_port= htons(atoi(argv[2]));
	struct hostent* hp= gethostbyname(argv[3]);
	//server_address.sin_addr.s_addr = inet_addr(argv[3]);	
	memcpy(&(server_address.sin_addr),hp->h_addr,hp->h_length);
	tryConnect(&client_socket);
	printf("Conectado a %s!!!!!!\n",inet_ntoa(server_address.sin_addr));
	
	//receber e armazenar dados recebidos
	char buff[PINGSIZE]={0};
	

	loginScreen();

        receiveWholeServerPing(buff,PINGSIZE);
        sscanf(buff,"%lu",&dataSize);
	printf("Tamanho de download; %lu bytes\n",dataSize);
	send(client_socket,buff,PINGSIZE,0);

	memset(buff,0,PINGSIZE);
	receiveWholeServerPing(buff,PINGSIZE);
        printf("%s\n",buff);
	//R S
	int currTotal=0;
	while(1){
		u_int64_t currDataSize;
		memset(buff,0,PINGSIZE);
		int status=receiveWholeServerPing(buff,PINGSIZE);
		if(status<0){
			printf("O client vai sair porque n recebeu um ping inteiro!!!!\n");
			raise(SIGINT);
		}
		sscanf(buff,"%lu",&currDataSize);
		snprintf(buff,PINGSIZE,"%s",pingCorrect);
		send(client_socket,buff,PINGSIZE,0);
		memset(buff,0,PINGSIZE);
	//	printf("Tamanhos:\ndados: %lu\n",dataSize);
	
	
		char message[currDataSize];
		memset(message,0,currDataSize);
	int total= receiveWholeServerPing(message,currDataSize);
		if(total<0){
			printf("O client vai sair porque n recebeu um chunk inteiro!!!!\n");
			raise(SIGINT);
		}
		//system("clear");
		status=write(fd,message,total);
		printf("Received chunk of data with size %d from %s\n\nWritting....to here:%s\n",status,inet_ntoa(server_address.sin_addr),argv[1]);
		if(status<0){
			perror("No bytes written!!!! An error happened\n");
		}
		currTotal+=status;
		
		printf("Done!!!!!\n");
		snprintf(buff,PINGSIZE,"%d",status);
		send(client_socket,buff,PINGSIZE,0);
		if(currTotal==dataSize){
			printf("Done!!!\n");
			raise(SIGINT);
		
		}
		
	}
	return 0;
}

