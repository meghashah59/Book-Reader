#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h> /* proc_create */
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system_misc.h> /* cli(), *_flags */
#include <linux/uaccess.h>
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/string.h>
#include <linux/sched.h> /* get user pid */
#include <linux/seq_file.h>
#include <linux/gpio.h>

#define BUFFER_SIZE 1024
#define BUTTON 17

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

struct timer_list timer;
static struct proc_dir_entry *proc_entry;
struct fasync_struct *async_queue; /* structure for keeping track of asynchronous readers */

static pid_t user_pid = -1;
bool user_is_ready = false;
bool button_press = false;
int time_in_s = 1;

static int manager_init(void)
{
	
	int result;
    unsigned long expires;
    const struct proc_ops *p_ops = (struct proc_ops*) &manager_fops;

	/* Registering device */
	result = register_chrdev(manager_major, "manager", &manager_fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"manager: cannot obtain major number %d\n", manager_major);
		return result;
	}

	proc_entry = proc_create("manager", 0444, NULL, p_ops);

    if (!proc_entry) {
        printk(KERN_ALERT "manager : Proc entry creation failed\n");
        return -ENOMEM;
    }	
	printk(KERN_ALERT "Inserting manager module\n"); 
	return 0;

    // gpio initialization
    if(gpio_request(BUTTON,"BUTTON") < 0) {
        printk(KERN_ALERT "ERROR: GPIO %d request\n",BUTTON);
        goto r_BUTTON;
    }
    gpio_direction_input(BUTTON);


    expires = jiffies + msecs_to_jiffies(time_in_s * 1000);
    memset(output_buffer, 0, BUFFER_SIZE);
    timer_setup(&timer, timer_callback, TIMER_DEFERRABLE);
    timer.expires = expires;
    add_timer(&timer);
    user_pid = current->pid;
    printk(KERN_INFO "Timer started\n");
	
r_BUTTON:
    gpio_free(BUTTON);
	
}

static void manager_exit(void)
{
	/* Freeing the major number */
	unregister_chrdev(manager_major, "manager");    	

    printk(KERN_INFO "Exiting my_module\n");

    del_timer(&timer);

	remove_proc_entry("manager", NULL);

    gpio_set_value(BUTTON,0);
    gpio_free(BUTTON);
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


static ssize_t manager_write(struct file *filp, const char *buf,
                             size_t count, loff_t *f_pos)
{
    	// Assuming buf contains the expiration time and message separated by a space
	

	if (copy_from_user(output_buffer, buf, count)!=0)
	{
		return -EFAULT;
	}

    if (output_buffer[0]=='1'){
        user_is_ready = true;
    }


    // Move the position for the next write
    *f_pos += count;

    return count;
}

static void timer_callback(struct timer_list *t)
{
	mod_timer(&timer, jiffies + msecs_to_jiffies(time_in_s*1000));
    if (gpio_get_value(BUTTON) == 1) 
        button_press = true;
    if (button_press && user_is_ready){
        button_press = false;
        user_is_ready = false;
        if (async_queue){
            kill_fasync(&async_queue, SIGIO, POLL_IN);
        }
	    user_pid = -1;
    }
	
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
