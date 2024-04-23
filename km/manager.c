#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system_misc.h> /* cli(), *_flags */
#include <linux/uaccess.h>
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/string.h>
#include <linux/sched.h> /* get user pid */
#include <linux/seq_file.h>

#define BUFFER_SIZE 1024
// #define TIMER_LIST_LENGTH 5

// struct my_timer_data {
//     struct timer_list timer;
// 	char message[128];
// };

/* Declaration of memory.c functions */
static int manager_open(struct inode *inode, struct file *filp);
static int manager_release(struct inode *inode, struct file *filp);
//static ssize_t manager_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
static ssize_t manager_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static void manager_exit(void);
static int manager_init(void);
static int manager_fasync(int fd, struct file *filp, int mode);
static void timer_callback(struct timer_list *t);
static int manager_proc_show(struct seq_file *m, void *v);

/* Structure that declares the usual file */
/* access functions */
struct file_operations manager_fops = {
	.read= seq_read,
	.write= manager_write,
	.open= manager_open,
	.llseek = seq_lseek,
	.release= manager_release,
	.fasync= manager_fasync
};



/* Declaration of the init and exit functions */
module_init(manager_init);
module_exit(manager_exit);



/* Global variables of the driver */
/* Major number */
static int manager_major = 61;

/* Buffer to store data */
static char output_buffer[BUFFER_SIZE];


int read_flag = 0;
// int max_timers = 1;
// char read_existing_timer[128];


struct timer_list timer;
// static struct my_timer_data timers[TIMER_LIST_LENGTH];
static struct proc_dir_entry *proc_entry;
struct fasync_struct *async_queue; /* structure for keeping track of asynchronous readers */

static unsigned long start_time;
static pid_t user_pid = -1;
char command[256];

static int manager_init(void)
{
	
	int result;

	/* Registering device */
	result = register_chrdev(manager_major, "manager", &manager_fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"manager: cannot obtain major number %d\n", manager_major);
		return result;
	}

	proc_entry = proc_create("manager", 0444, NULL, &manager_fops);

    if (!proc_entry) {
        printk(KERN_ALERT "manager : Proc entry creation failed\n");
        return -ENOMEM;
    }
	start_time = jiffies;

	
	printk(KERN_ALERT "Inserting manager module\n"); 
	return 0;

	
}

static void manager_exit(void)
{
	/* Freeing the major number */
	unregister_chrdev(manager_major, "manager");    	

    printk(KERN_INFO "Exiting my_module\n");

    del_timer(&timer);

	remove_proc_entry("manager", NULL);
}

static int manager_open(struct inode *inode, struct file *filp)
{
	//printk(KERN_INFO "open called: process id %d, command %s\n",
	//	current->pid, current->comm);
	/* Success */

	return single_open(filp, manager_proc_show, NULL);
}


static int manager_release(struct inode *inode, struct file *filp)
{
	//printk(KERN_INFO "release called: process id %d, command %s\n",
		//current->pid, current->comm);
	memset(output_buffer,0,sizeof(BUFFER_SIZE));
	manager_fasync(-1, filp, 0);
	single_release(inode, filp);
	


	// Success 
	return 0;
}


/*
static ssize_t manager_read(struct file *filp, char *buf, 
							size_t count, loff_t *f_pos)
{ 
	//struct timer_info *timer_data;
    	
    	
    int bytes_read = 0;
	int buffer_pos = 0;

	memset(output_buffer, 0, BUFFER_SIZE);
	
	if (read_flag ==0) {
		int i;
		for (i = 0; i < TIMER_LIST_LENGTH; ++i) {
			struct my_timer_data *data = &timers[i];
			if (strcmp(data->message, "") != 0) {
		
				//printk(KERN_INFO "Message %s and time remaining %ld\n", data->message, get_time_remaining(data));
				int written = snprintf(output_buffer+buffer_pos, sizeof(output_buffer) - buffer_pos, "%s %ld\n",data->message,get_time_remaining(data));
		

				// Update the total bytes_read
				buffer_pos += written; //strlen(output_buffer + buffer_pos);

				// Check if there is enough space in the buffer
				if (buffer_pos >= BUFFER_SIZE)
					break;  // Stop appending if the buffer is full
			
			}
    	}

		//printk(KERN_INFO "%s\n", output_buffer);

    		// Copy the formatted string to user space
		if (*f_pos ==0) {
			bytes_read = min(count, strlen(output_buffer));
			if (copy_to_user(buf, output_buffer, BUFFER_SIZE) != 0)
			{
				return -EFAULT;
			}
			*f_pos += bytes_read;//buffer_pos;
			//bytes_read = buffer_pos;
		
		}

    	// Reset the position for the next read
    	
    	return bytes_read;
	}

	else if (read_flag ==1) {
		int written2=snprintf(output_buffer+buffer_pos, sizeof(output_buffer) - buffer_pos, "The timer %s was updated!\n", read_existing_timer);
		buffer_pos += written2;

		bytes_read = min(count, strlen(output_buffer));
		if (copy_to_user(buf, output_buffer, BUFFER_SIZE) != 0)
		{
			return -EFAULT;
		}
		*f_pos += bytes_read;//buffer_pos;


    	// Reset the position for the next read
		read_flag=0;
    	
    	return bytes_read;
	}
	else if (read_flag ==2) {
		int written2=snprintf(output_buffer+buffer_pos, sizeof(output_buffer) - buffer_pos, "%d timer(s) already exist(s)!\n", max_timers);
		buffer_pos += written2;

		bytes_read = min(count, strlen(output_buffer));
		if (copy_to_user(buf, output_buffer, BUFFER_SIZE) != 0)
		{
			return -EFAULT;
		}
		*f_pos += bytes_read;//buffer_pos;


    	// Reset the position for the next read
		read_flag=0;
    	
    	return bytes_read;
	}

	 
}
*/

static ssize_t manager_write(struct file *filp, const char *buf,
                             size_t count, loff_t *f_pos)
{
    	// Assuming buf contains the expiration time and message separated by a space
	unsigned long expires;
    expires = jiffies + msecs_to_jiffies(10 * 1000);
    memset(output_buffer, 0, BUFFER_SIZE);
    timer_setup(&timer, timer_callback, TIMER_DEFERRABLE);
    timer.expires = expires;
    add_timer(&timer);
    user_pid = current->pid;
    printk(KERN_INFO "Timer 10s started\n");
	

	if (copy_from_user(output_buffer, buf, count)!=0)
	{
		return -EFAULT;
	}


    // Move the position for the next write
    *f_pos += count;

    return count;
}

static void timer_callback(struct timer_list *t)
{
	del_timer(&timer);

	if (async_queue)
        kill_fasync(&async_queue, SIGIO, POLL_IN);
	
	user_pid = -1;
    // printk(KERN_INFO "%s\n", data->message);
}


static int manager_fasync(int fd, struct file *filp, int mode) {
    return fasync_helper(fd, filp, mode, &async_queue);
}

static int manager_proc_show(struct seq_file *m, void *v)
{

    seq_printf(m, "manager- name of the module\n");

    if (user_pid != -1) {
        seq_printf(m, "%d- user pid\n", user_pid);
    }

    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Megha Shah");
MODULE_DESCRIPTION("Manager");
