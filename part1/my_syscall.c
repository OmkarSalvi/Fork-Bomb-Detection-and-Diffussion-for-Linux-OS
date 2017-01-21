/* syscall source code */
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/cred.h>


asmlinkage long sys_my_syscall(int a, int b, char *c)
{
	char buf[100];

	int t_timesecs,t_secs,t_mins,t_hrs;
	int n=0, count=0, len=0; //count keeps track of number of processes

	struct task_struct *task;
	struct signal_struct *signalx;
	struct tty_struct *ttyx;
	
	cputime_t utime = 0;
 	cputime_t stime = 0;

	for_each_process(task)
        {	
		if(a==0)
		{
			count++;
		}
		else
		{
			count++;
			if(count==a)
			{
				signalx=task->signal;
				ttyx=signalx->tty;
				utime = 0;
 				stime = 0;
				thread_group_cputime_adjusted(task, &utime, &stime);
				t_timesecs = (utime + stime)/HZ;
				t_hrs=t_timesecs/3600;
				t_mins=(t_timesecs%3600)/60;
				t_secs=(t_timesecs%3600)%60;
				if(!ttyx)
				{
					sprintf(buf,"%5d %s\t\t%02d:%02d:%02d %s",task->pid,"?", t_hrs, t_mins, t_secs, task->comm);
				}
				else 
				{
					sprintf(buf,"%5d %s\t%02d:%02d:%02d %s",task->pid, ttyx->name, t_hrs, t_mins, t_secs, task->comm);	
				}
				break;
			}
			
		}
	}
	len=strlen(buf);
	if(a==0)
	{
		return count;
	}
	else if(a==count)
	{
		n=copy_to_user(c,buf,len);
		return n;
	}
	else if(a>count)			//To take care of processes getting killed in realtime.
	{
		return -1;
	}
	
	return n;
}

