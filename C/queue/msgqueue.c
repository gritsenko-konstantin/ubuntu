#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> /*for msgqueue*/
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#define IPC_KEY 6666
struct msgbuf {
long mtype; /* msgtype */ 
char mtext[1]; /* msgcontext */
};
int main(int args,char* argv[])
{
	int iRet = 0,imsgkey = 0;
	struct msgbuf msg;
	/*0660 is read and write auth-access
	IPC_CREAT if queue existed,then get it ,otherwise create it;
	IPC_EXCL if queue existed return error,used with IPC_CREAT*/	
	imsgkey = msgget(IPC_KEY,IPC_CREAT|0660);
	if(imsgkey < 0 )
	{
		fprintf(stdout,"msgget error errno[%d]=[%s]",errno,strerror(errno));
	}
	alarm(1);
	signal(SIGALRM,NULL);
	iRet = msgrcv(imsgkey,&msg,sizeof(msg)-sizeof(long),0,MSG_NOERROR);
	alarm(0);
	if(iRet == -1)
	{
		fprintf(stdout,"msgrcv timeout errno[%d]=[%s]",errno,strerror(errno));
	}	
	return 0;
	
}
