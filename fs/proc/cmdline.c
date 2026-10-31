#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

static int cmdline_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", saved_command_line);
	return 0;
}

static int cmdline_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cmdline_proc_show, NULL);
}

/*
 * The stock Samsung kernel allows /proc/cmdline to be overwritten so
 * that the FeliCa provisioning values (cordon/connie) restored from
 * /data/media/0/kbc/cmdline by init.felica.sh take effect for readers
 * such as mfsc, whose custom-kernel check compares the live command
 * line against the provisioned one.  The stock command line buffer
 * is large enough for this: writes are bounded by the length of the
 * command line the bootloader originally supplied.
 */
static ssize_t cmdline_proc_write(struct file *file,
		const char __user *buf, size_t count, loff_t *ppos)
{
	size_t len = strlen(saved_command_line);

	if (count > len)
		count = len;
	if (copy_from_user(saved_command_line, buf, count))
		return -EFAULT;
	saved_command_line[count] = '\0';
	return count;
}

static const struct file_operations cmdline_proc_fops = {
	.open		= cmdline_proc_open,
	.read		= seq_read,
	.write		= cmdline_proc_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_cmdline_init(void)
{
	proc_create("cmdline", 0660, NULL, &cmdline_proc_fops);
	return 0;
}
module_init(proc_cmdline_init);
