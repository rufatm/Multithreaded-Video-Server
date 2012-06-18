/*
 * kernel module containing the definitoins of my mutex and condition 
 * Author: Rufat Mammadyarov
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> 
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/semaphore.h> 


MODULE_LICENSE("GPL");
#include "Leonardo.h"

static struct proc_dir_entry *proc_entry; // the thing to register the ioctl

static struct semaphore mutex_array[500];  // store the mutexes
int m_last_index;  // last index of mutex array


static struct semaphore condition_array[500];  // store conditional variables
int c_last_index; // last index of the condition array





struct semaphore splinter; // why not


static int handle_ioctl(struct inode *, struct file *, unsigned int , unsigned long ) ;
static int handle_mutex(struct mutex_struct*);
static int handle_condition(struct condition_struct*); 


// Thanks to Rich for that
static int __init initialization_routine(void) {
  printk("<1> Loading module\n");
  static struct file_operations pseudo_dev_proc_operations; 
  pseudo_dev_proc_operations.ioctl = handle_ioctl;
  m_last_index = 0; 
  

  /* Start create proc entry */
  proc_entry = create_proc_entry("Leonardo", 0444, NULL);
  if(!proc_entry)
  {
    printk("<1> Error creating /proc entry.\n");
    return 1;
  }

  //proc_entry->owner = THIS_MODULE; <-- This is now deprecated
  proc_entry->proc_fops = &pseudo_dev_proc_operations;
  sema_init(&splinter,1);
  return 0;
}

static void __exit cleanup_routine(void) 
{ 
	remove_proc_entry("Leonardo", NULL);

}







static int handle_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
{ 

  switch(cmd) 
  { 
	case (MUTEX): 
		return handle_mutex((struct mutex_struct*) arg);
		break;
	case (CONDITION): 
		return handle_condition((struct condition_struct*) arg);
		break;
	default: 
		printk("Something went terribly wrong in the kernel");
		return -1; // ? 
		break;
  }
}


static int handle_mutex(struct mutex_struct* var_mutex) 
{ 


	if (var_mutex->action == 0) // init
	{ 
		var_mutex->id  = m_last_index;
		int n = var_mutex->test;
		
		// initialization... adding to the array.. I'm not sure
		sema_init(&mutex_array[m_last_index++],n); 
		return 0;
	}

	else if (var_mutex->action  == 1)  // lock
	{
		//int res = EINTR;
		//while (res == EINTR)
			down_interruptible(&mutex_array[var_mutex->id]);
		return 0;
	}
	else if (var_mutex->action == 2) // unlock
	{ 
		up(&mutex_array[var_mutex->id]);
		return 0;	
	}
	return -1;
}


static int handle_condition(struct condition_struct* var_condition) 
{ 

	if(var_condition->action == 0) 
	{
		var_condition->sleeper.id = m_last_index;
		sema_init(&mutex_array[m_last_index++],0);
		return 0;
	} 

	else if (var_condition->action == 1) /// wait 
	{ 
		up(&mutex_array[var_condition->rafael.id]);
		down_interruptible(&mutex_array[var_condition->sleeper.id]); 
		down_interruptible(&mutex_array[var_condition->rafael.id]);
		return 0;	
	}
	else if (var_condition == 2) 
	{
		while (mutex_array[var_condition->sleeper.id].count)
		up(&mutex_array[var_condition->rafael.id]);	
	} 
}




module_init(initialization_routine); 
module_exit(cleanup_routine); 


