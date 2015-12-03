/*
 *	PR 4 linux 
 *	Used 4 packet redirect 
 *	Codz By huigou
 *	Compile: gcc -o lcx lcx.c
 *	5/11/2015   4 edu only!
 */

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/*#define DEBUG*/

#define ERROR_Q(x) {	printf("%s failed!\n",x); exit(1);}
#define ERROR_B(x) {    printf("%s failed!\n",x); break;}

void * TcpDataC2T(int *);

void * TcpDataT2C(int *);


int tranpart(int argc,char **argv)
{

	int localfd,sock[2],port;
	struct sockaddr_in local,client,target;
	int iAddrSize;
	
	
	port =atoi(argv[2]);
	if((localfd = socket(AF_INET,SOCK_STREAM,0))<0)
		ERROR_Q("socket");

	local.sin_family = AF_INET;
	local.sin_port   = htons(port);
        local.sin_addr.s_addr = htonl(INADDR_ANY);

	target.sin_family = AF_INET;
	target.sin_port   = htons(atoi(argv[4]));
	target.sin_addr.s_addr = inet_addr(argv[3]);

	if(bind(localfd,(struct sockaddr *)&local
			,sizeof(local))<0)

		ERROR_Q("bind");	


	if(listen(localfd,5)<0)
		ERROR_Q("listen");

	while(1){
	#ifdef DEBUG
		printf("\n\n*********Waiting Client connect*****\n\n");
	#endif
		iAddrSize=sizeof(client);
		
		sock[0] = accept(localfd,(struct sockaddr *)&client
				,&iAddrSize);
		if(sock[0]<0)
			ERROR_B("accept");
	#ifdef DEBUG
		printf("\nAccept Client ==> %s:%d"
				,inet_ntoa(client.sin_addr)
				,ntohs(client.sin_port));
	#endif
	
		if((sock[1]=socket(AF_INET,SOCK_STREAM,0))<0)
			ERROR_Q("sock[1]");
		
		if(connect(sock[1],(struct sockaddr*)&target
					,sizeof(target))<0)
			ERROR_Q("connect to target");
		
	#ifdef DEBUG
		printf("connect to target succfully!\n");
	#endif
		if(fork()==0){
			
			 TcpDataC2T(sock);
			 shutdown(sock[0],1);
			shutdown(sock[1],1);
			 exit(0);
		}

		if(fork()==0){
		
			TcpDataT2C(sock);
			shutdown(sock[0],1);
			shutdown(sock[1],1);
			exit(0);
		}
		
		close(sock[0]);
		close(sock[1]);
		
	}

	exit(0);
}


int listenpart(int argc,char * argv[])
{
	
	int listenfd,tranfd,sock[2];

	struct sockaddr_in local,tran,client,slave;

	int iAddrSize;
#ifdef DEBUG
	printf("\nin listen!\n");
#endif
	if((listenfd = socket(AF_INET,SOCK_STREAM,0))<0)
		ERROR_Q("socket listenfd");
	if((tranfd = socket(AF_INET,SOCK_STREAM,0))<0)
		ERROR_Q("socket tranfd");

	local.sin_family = AF_INET;
	local.sin_port  = htons(atoi(argv[3]));
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	
	tran.sin_family	=AF_INET;
	tran.sin_port   = htons(atoi(argv[2]));
	tran.sin_addr.s_addr  = htonl(INADDR_ANY);

	if(bind(listenfd,(struct sockaddr*)&local
			,sizeof(local))<0)
		ERROR_Q("bind listenfd");

	if(bind(tranfd,(struct sockaddr*)&tran
			,sizeof(tran))<0)

		ERROR_Q("bind tranfd");

	if(listen(tranfd,5)<0)
		ERROR_Q("listen tranfd");

	if(listen(listenfd,5)<0)
		ERROR_Q("listen listenfd");

	while(1){
	
	
		iAddrSize =sizeof(slave);
		sock[1] = accept(tranfd,(struct sockaddr*)&slave
				,&iAddrSize);
		if(sock[1]<0)
			ERROR_B("sock[1] accept");
#ifdef DEBUG
		printf("\nOh! there come a slave!\n");
#endif
		sock[0] = accept(listenfd,(struct sockaddr*)&client
				,&iAddrSize);

		if(sock[0]<0)
			ERROR_Q("sock[0] accept");
#ifdef DEBUG
		printf("\nMaster has logged in!\n");
#endif		
		if(fork()==0){
			
			 TcpDataC2T(sock);
			 shutdown(sock[0],1);
			 shutdown(sock[1],1);
			 exit(0);
			 
		}
		 if(fork()==0){
		
			TcpDataT2C(sock);
			 shutdown(sock[0],1);
			 shutdown(sock[1],1);
			exit(0);
		}
	   }
		
		

	exit(0)	;
}


int slavepart(int argc,char * argv[])
{
	int sock[2];

	struct sockaddr_in client ,target;
	pid_t pid1,pid2;

	while(1){
	
		if((sock[0]=socket(AF_INET,SOCK_STREAM,0))<0)
			ERROR_Q("socket[0] create");

		if((sock[1]=socket(AF_INET,SOCK_STREAM,0))<0)
			ERROR_Q("socket[1] create");

		client.sin_family = AF_INET;
		client.sin_port = htons(atoi(argv[3]));
		client.sin_addr.s_addr = inet_addr(argv[2]);

		target.sin_family = AF_INET;
		target.sin_port =htons(atoi(argv[5]));
		target.sin_addr.s_addr =inet_addr(argv[4]);


		//connect to client
		if(connect(sock[0],(struct sockaddr*)&client
					,sizeof(client))<0)
			
			ERROR_Q("connect to client");
#ifdef DEBUG	
		printf("\nconnect to client successfully!\n");
#endif
		if(connect(sock[1],(struct sockaddr*)&target
					,sizeof(target))<0)
			ERROR_Q("connect to target");
#ifdef DEBUG
		printf("\nconnect to target successfully\n");
#endif
		if((pid1=fork())==0){
			
			 TcpDataC2T(sock);
			 shutdown(sock[0],1);
			 shutdown(sock[1],1);
			 exit(0);
		}

		if((pid2=fork())==0){
		
			TcpDataT2C(sock);
			shutdown(sock[0],1);
		        shutdown(sock[1],1);
			exit(0);
		}
		
	
		waitpid(pid1,NULL,0);
		waitpid(pid2,NULL,0);	
	}

	exit(0);
}


void * TcpDataC2T(int * sock)
{
	
	
	int iRet,
	    ret=-1,
	    iLeft,idx,iSTTBCS=0;
	char szSendToTargetBuff[BUFSIZ]={0},
	     szRecvFromClientBuff[BUFSIZ]={0};

	fd_set fdread,fdwrite;
#ifdef DEBUG
	printf("TcpDataC2T started!\n\n");
	printf("\n\n**********Connection Active******\n\n");
#endif
	while(1){
	
	
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_SET(sock[0],&fdread);
		FD_SET(sock[1],&fdwrite);

		if((ret=select(sock[0]>sock[1]?sock[0]+1:sock[1]+1
				,&fdread,&fdwrite,NULL,NULL))<0)
			ERROR_B("\nselect ");

		if(ret>0){
		
			if(FD_ISSET(sock[0],&fdread)){
			/*Date from client coming*/
				
				if((iRet = recv(sock[0]
				,szRecvFromClientBuff,BUFSIZ,0))<0)
					ERROR_B("\nrecv from client");
				if(iRet==0)
					return (0);

#ifdef DEBUG					
				
				printf("recv %d from client\n",iRet);
#endif
				memcpy(szSendToTargetBuff+iSTTBCS
					,szRecvFromClientBuff,iRet);
				
				iSTTBCS +=iRet;

				memset(szRecvFromClientBuff,0,BUFSIZ);				
			}

			/*write to target if possible*/
			if(FD_ISSET(sock[1],&fdwrite)){
				
				iLeft=iSTTBCS;
				idx=0;
				
				while(iLeft>0){
				  iRet=send(sock[1]
				    ,&szSendToTargetBuff[idx],iLeft,0);

				  if(iRet<0)
					  ERROR_B("\n send() to "
					    "target");
#ifdef DEBUG
				  printf("Send %d bytes to target!\n",iRet);
#endif
				  iLeft -=iRet;
				  idx+=iRet;
				
				
				}
			
				memset(szSendToTargetBuff,0,BUFSIZ);
				iSTTBCS=0;
			}
		
		}
		
			//sleep(1);

	
	}	
	
		return 0;
	
}

void * TcpDataT2C(int *sock)
{
#ifdef DEBUG
	printf("TCPDATAT2C started!\n");
#endif
	
	int iRet,
	    ret=-1,
	    iLeft,idx,iSTTBCS=0;
	char szSendToClientBuff[BUFSIZ]={0},
	     szRecvFromTargetBuff[BUFSIZ]={0};

	fd_set fdread,fdwrite;
#ifdef DEBUG
	printf("\n\n**********Connection Active******\n\n");
#endif	
	while(1){
	
	
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_SET(sock[1],&fdread);
		FD_SET(sock[0],&fdwrite);

		if((ret=select(sock[0]>sock[1]?sock[0]+1:sock[1]+1
				,&fdread,&fdwrite,NULL,NULL))<0)
			ERROR_B("\nselect ");

		if(ret>0){
		
			if(FD_ISSET(sock[1],&fdread)){
			/*Date from target coming*/
				
				if((iRet = recv(sock[1]
				,szRecvFromTargetBuff,BUFSIZ,0))<0)
					ERROR_B("\nrecv from target");
				
				if(iRet==0)
					return(0);
#ifdef DEBUG
				printf("recv %d from target\n",iRet);
#endif
		
				memcpy(szSendToClientBuff+iSTTBCS
					,szRecvFromTargetBuff,iRet);
				
				iSTTBCS +=iRet;

				memset(szRecvFromTargetBuff,0,BUFSIZ);				
			}

			/*write to target if possible*/
			if(FD_ISSET(sock[0],&fdwrite)){
				
				iLeft=iSTTBCS;
				idx=0;
				
				while(iLeft>0){
				  iRet=send(sock[0]
				    ,&szSendToClientBuff[idx],iLeft,0);

	

				  if(iRet<0)
					  ERROR_B("\n send() to "
					    "client");
#ifdef DEBUG
				  printf("Send %d bytes to client!\n",iRet);
#endif
				  iLeft -=iRet;
				  idx+=iRet;
				
				
				}
			
				memset(szSendToClientBuff,0,BUFSIZ);
				iSTTBCS=0;
				}
			}	
			//sleep(1);
		}


	return 0;		
}



	
 
 void usage()
{
	printf(" PR(Packet redirection) for linux \n"
			"Codz by huigou\n\n"
			"Usage: Same as lcx.exe in win32 :)\n");
	printf("lcx -listen <connectPort> <TransmitPort>\n");
	printf("lcx -tran <connectPort> <TransmithHost> <TransmithPort>\n");
	printf("lcx -slave <connectHost> <ConnectPort> <TransmithPort> <TransmithPort>\n\n");

	exit(0);
}


 int main(int argc ,char * argv[])
{
	
	if(argc>=4){
		if(!strcmp(argv[1],"-listen"))
			listenpart(argc,argv);

		else if(!strcmp(argv[1],"-tran"))
			tranpart(argc,argv);
	
		else if (!strcmp(argv[1],"-slave"))
			slavepart(argc,argv);
	}
	usage();

	exit(0);
}


