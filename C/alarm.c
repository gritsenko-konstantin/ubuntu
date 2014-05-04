#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
void sigpro();
int main(int args,char *argv[])
{
	alarm(30);	
	sleep(10);
	int iRet = alarm(10);	
	printf("alarm time[%d] \n",iRet);
	/*if not get SIGALRM,process will exit*/
	/*SIG_IGN,continue the program,sigpro will be called,and then the program will go on*/
	signal(SIGALRM,sigpro);	
	/*signal(SIGALRM,SIG_IGN);*/
	sleep(30);
	int i = 0;
	for(i = 0;i < 5; i++)
	{
		sleep(1);
		printf("[%d]\n",i);
	}	
	return 0;
}
void sigpro()
{
	fprintf(stdout,"time out event happened \n");
}
