/*User space program to test the system call*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define __NR_my_syscall 359

int main()
{	char buff[100];
	int i,count=0,returnvalue;	//Process count
	count=syscall(__NR_my_syscall,0,0,buff);
	printf("%5s %s%15s%4s\n","PID","TTY","TIME","CMD");
	for(i = 1; i <= count; i++)
	{	
		memset(buff,0,100);	
		returnvalue=syscall(__NR_my_syscall,i,0,buff);
		if(returnvalue==-1) //For handling processes getting killed in realtime		
			break; //Break out of the loop..you have printed all the currently running processes 						
		printf("%s\n",buff);
		//strcpy(buff,'\0');		
	}
	return 0;
}
