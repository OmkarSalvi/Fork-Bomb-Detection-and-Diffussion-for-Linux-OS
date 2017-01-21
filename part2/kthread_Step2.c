#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>	   // Kmalloc/Kfree
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/semaphore.h>

//SB: If the number of processes created by fork() exceeds THRESHOLD, defusing action will start.
#define THRESHOLD	100

//SB: Use semaphore exported from fork.c file in kernel
extern struct semaphore *sem_fork_e;

static int Monitor_Fork_Bomb(void *data);
//SB: Declare and initialize kernel thread to detect fork bomb
static struct task_struct * KernelDetectThread = NULL;


/* 
 * VSK: Does depth first search to locate all children
 */
int detect(struct task_struct *task,int print)
{	
	int count=0;
	struct list_head *list;
	struct task_struct *another_task;
	list_for_each(list, &task->children) 
	{
    		count++;
		another_task = list_entry(list, struct task_struct, sibling);
		if(print==1)				//VSK: will print apart from counting when print==1
			{
				printk("Detected Fork Bomb Process PID: %d\n",another_task->pid);
				count+=detect(another_task,1);
			}
		else					//VSK: only counts otherwise..doesnt print
			count+=detect(another_task,0);
 	}
	return count;
}


/*
 * Defuses fork bomb processes
 */
void defuser(struct task_struct *task)
{
	struct list_head *list;
	struct task_struct *another_task;
	list_for_each(list, &task->children) 
	{
		another_task = list_entry(list, struct task_struct, sibling);
		defuser(another_task);

 	}
	
	printk("Killed Fork Bomb Process PID: %d\n",task->pid);
	send_sig_info(SIGKILL, SEND_SIG_FORCED, task);	
}


/*
 * VSK: Runs through each task and calls detect() function to count no. of children of each task and then 
 * calls defuser() to kill that task which has more children than the threshold.
 */
void wrapper_detect(void)
{				
	struct task_struct *task;
	int count=0;
	for_each_process(task)
	{
		if(!(task->pid==0 || task->pid==1 || task->pid==2 || !(strcmp(task->comm,"init")) || !(strcmp(task->comm,"bash")) || !(strcmp(task->comm,"gnome-session")) || !(strcmp(task->comm,"lightdm")) || !(strcmp(task->comm,"gnome-terminal"))))
		{
			count=detect(task,0);
		}		
		
		if(count>=(THRESHOLD-1)) 		// VSK: Threshold number
		{
			printk("Detected Fork Bomb Process PID: %d\n", task->pid);
			detect(task,1);		// VSK: This function is being called here only for the purpose of printing PIDS of the fork bomb		
			defuser(task);		
			break;
		}
	}
	printk("\n\n");
}
			

/*
 * SB: Kernel Thread Handler to monitor Fork Bomb
 */
static int Monitor_Fork_Bomb(void *data)
{
	while(!kthread_should_stop())
	{
		down(sem_fork_e); // SB : Locks Semaphore Mutex
		wrapper_detect();
	}

	return 0;
}

/*
 * SB: Fork Bomb Detection Kernel Module Init Function
 */
static int __init detect_init(void)
{
	KernelDetectThread = kthread_run(Monitor_Fork_Bomb, NULL,"Kernel_Thread_Detect_ForkBomb");
	return 0;
}


/*
 * SB: Fork Bomb Detection Kernel Module Exit Function
 */
static void __exit detect_exit(void)
{
	//SB: Stop the kernel thread as exiting from kernel module
	up(sem_fork_e); // SB: To exit from the thread safely
	if(KernelDetectThread)
	{
		kthread_stop(KernelDetectThread);
	}
}

module_init(detect_init);
module_exit(detect_exit);
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("SAILI BAKARE, SIVAKUMAR VENKATARAMAN, OMKAR SALVI");
MODULE_DESCRIPTION("Project 2 Step 2 for CSE430 OPERATING SYSTEMS");
