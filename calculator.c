#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>

#define FIRST_OP_SIZE	10
#define SECOND_OP_SIZE	10
#define	OP_SIZE		2

static char first[10];
static char second[10];
static char answer[20];
static char operation[2];

static int first_op;
static int second_op;
static int answer_op;
static int base = 10;

static int len_f, temp_f;
static int len_s, temp_s;
static int len_o, temp_o;

int i;

static const char *dirname = "calculator";
static const char *first_operand = "first_operand";
static const char *second_operand = "second_operand";
static const char *operation_name = "operation";
static const char *answer_name = "answer";
static const char *error = "invalid operation\n";
static const char *zero_div = "division by zero\n";
static struct proc_dir_entry *parent;

/*********************FIRST OPERAND************************/
static ssize_t read_first_operand (struct file *file, char *buf, size_t count, loff_t *offp)
{
	if(count > temp_f)
	{
		count = temp_f;
	}
	temp_f -= count;
	copy_to_user(buf, &first, count);
	if(count == 0)
		temp_f = len_f;
	return count;
}

static ssize_t write_first_operand (struct file *file, const char *buf, size_t count, loff_t *offp)
{
	for(i = 0; i < FIRST_OP_SIZE; i++)
		first[i] = 0;
	
	printk(KERN_ERR "%d", count);
	copy_from_user(&first, buf, count);
	len_f = count;
	temp_f = len_f;
	return count;
}

static struct file_operations first_operand_fops = {
	.owner = THIS_MODULE,
	.read  = read_first_operand,
	.write = write_first_operand
};

/*********************SECOND OPERAND************************/
static ssize_t read_second_operand (struct file *file, char *buf, size_t count, loff_t *offp)
{
	if(count > temp_s)
	{
		count = temp_s;
	}
	temp_s -= count;
	copy_to_user(buf, &second, count);
	if(count == 0)
		temp_s = len_s;
	return count;
}

static ssize_t write_second_operand (struct file *file, const char *buf, size_t count, loff_t *offp)
{
	for(i = 0; i < SECOND_OP_SIZE; i++)
		second[i] = 0;	

	copy_from_user(&second, buf, count);
	len_s = count;
	temp_s = len_s;
	return count;
}

static struct file_operations second_operand_fops = {
	.owner = THIS_MODULE,
	.read  = read_second_operand,
	.write = write_second_operand
};

/*********************OPERATION************************/
static ssize_t read_operation (struct file *file, char *buf, size_t count, loff_t *offp)
{
	if(count > temp_o)
	{
		count = temp_o;
	}
	temp_o -= count;
	copy_to_user(buf, &operation, count);
	if(count == 0)
		temp_s = len_o;
	return count;
}

static ssize_t write_operation (struct file *file, const char *buf, size_t count, loff_t *offp)
{
	if(count > OP_SIZE)
		count = OP_SIZE;

	copy_from_user(&operation, buf, count);
	len_o = count;
	temp_o = len_o;
	return count;
}

static struct file_operations operation_fops = {
	.owner = THIS_MODULE,
	.read  = read_operation,
	.write = write_operation
};

/***********************ANSWER*************************/
static ssize_t read_answer (struct file *file, char *buf, size_t count, loff_t *offp)
{
	int i;
	static int finished = 0;
	if(finished)
	{
		finished = 0;
		return 0;
	}

	kstrtoint(first, base, &first_op);
	kstrtoint(second, base, &second_op);

	switch(operation[0])
	{
		case '+':
			answer_op = first_op + second_op;
			break;
		case '-':
			answer_op = first_op - second_op;
			break;
		case '*':
			answer_op = first_op * second_op;
			break;
		case '/':
			if(second_op == 0)
			{
				copy_to_user(buf, zero_div, strlen(zero_div));
				finished = 1;
				return (strlen(zero_div));
			}
			answer_op = first_op / second_op;
			break;
		default:
			copy_to_user(buf, error, strlen(error));
			finished = 1;
			return (strlen(error));
			break;
	}

	sprintf(answer, "%d %c %d = %d\n\0", first_op, operation[0], second_op, answer_op);

	for(i = 0; answer[i] != '\0'; i++)	
		put_user(answer[i], buf+i);

	finished = 1;
	return i;
}

static struct file_operations answer_fops = {
	.owner = THIS_MODULE,
	.read  = read_answer
};

static int __init calculator_init (void)
{
	parent = proc_mkdir(dirname, NULL);
	proc_create(first_operand, S_IRWXUGO, parent, &first_operand_fops);
	proc_create(second_operand, S_IRWXUGO, parent, &second_operand_fops);
	proc_create(operation_name, S_IRWXUGO, parent, &operation_fops);
	proc_create(answer_name, S_IRWXUGO, parent, &answer_fops);
	return 0;
}

static void __exit calculator_exit (void)
{
	remove_proc_entry(first_operand, parent);
	remove_proc_entry(second_operand, parent);
	remove_proc_entry(operation_name, parent);
	remove_proc_entry(answer_name, parent);
	proc_remove(parent);
}

module_init(calculator_init);
module_exit(calculator_exit);
	

