#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
int main(int args,char *argv[])
{
	pid_t childpid;
	childpid = fork();
	if(childpid < 0 )
		fprintf(stderr,"fork child process error \n");
	
	if(0 == childpid)
	{
		sleep(5);
		fprintf(stdout,"in child process \n");
		exit(0);
	}
	if(childpid > 0)
	{
		fprintf(stdout,"in parents process [%d] \n",childpid);
		signal(SIGCLD,SIG_IGN);/*in order to avoid child process to be a defunct process */
		sleep(30);	
		fprintf(stdout,"in parents process \n");
		exit(0);
	}
}
