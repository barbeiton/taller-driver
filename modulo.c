#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define MAX 1
static char *estado;
static char x;
static char y;
static int movimiento_imposible(char x, char y, char mov) {
	int imposible = 0;
	switch(mov) {
		case 'N':
			imposible = y == 0 ? 1:0;
			break;
		case 'S':
			imposible = y == 9 ? 1:0;
			break;
		case 'E':
			imposible = x == 9 ? 1:0;
			break;
		case 'W':
			imposible = x == 0 ? 1:0;
			break;
		default:
			printk(KERN_ALERT "Input incorrecto\n");
			return -1;	
	}
	return imposible;
}

static void actualizar_estado(char *estado, char x, char y) {
	estado[2] = '0'+x;
	estado[4] = '0'+y;
}

static dev_t device_no;
static struct file_operations operaciones;
static struct cdev device;
static struct class *mod_class;

//Ejercicio 2
ssize_t read_op(struct file *filp, 
		char __user *data, 
		size_t s, 
		loff_t *off) {
	
	printk(KERN_ALERT "Llamada a read\n");
	printk(KERN_ALERT "estado: %s", estado);
	copy_to_user(data, estado, 10);
	return 0;
}

//Ejercicio 3b
ssize_t write_op(struct file *filp, 
		const char __user *data, 
		size_t s, 
		loff_t *off) {
	
	char buffer[MAX+1];
	buffer[MAX] = 0;

	printk(KERN_ALERT "Llamada a write\n");

	if (MAX < s) {
		printk(KERN_ALERT "Input incorrecto\n");
		return 1;
	}

	if (copy_from_user(buffer, data, 1) != 0) {
		printk(KERN_ALERT "copy_from_user error en write\n");
		return 1;
	}

	if (movimiento_imposible(x, y, buffer[0])) {
		printk(KERN_ALERT "Movimiento imposible\n");
		return 1;
	}

	//printk(KERN_ALERT "%s\n", buffer);
	switch(buffer[0]) {
		case 'N':
			y--;
			break;
		case 'S':
			y++;
			break;
		case 'E':
			x++;
			break;
		case 'W':
			x--;
			break;
		default:
			printk(KERN_ALERT "Input incorrecto\n");
			return -1;		
	}
	actualizar_estado(estado, x, y);
	return 1;
}

static int __init mod_init(void) {
	printk(KERN_ALERT "Cargando modulo!\n");
	//Ejercicio 1
	estado = kmalloc(10, GFP_KERNEL);
	strncpy(estado, "A 0-0: _\n", 9);
	estado[9] = 0;
	x = 0, y = 0;
		
	//Ejercicio 3a
	alloc_chrdev_region(&device_no, 0, 1, "CualquierNombre?");
	
	operaciones.owner = THIS_MODULE;
	operaciones.read = read_op;
	operaciones.write = write_op;
	cdev_init(&device, &operaciones);
	cdev_add(&device, device_no, 1);

	mod_class = class_create(THIS_MODULE, "modulo-taller");
	device_create(mod_class, NULL, device_no, NULL, "modulo-taller");

	return 0;
}

static void __exit mod_exit(void) {
	printk(KERN_ALERT "Descargando modulo\n");
	device_destroy(mod_class, device_no);
	class_destroy(mod_class);
	cdev_del(&device);
	unregister_chrdev_region(device_no, 1);
	kfree(estado);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AUTOR");
MODULE_DESCRIPTION("DESCRIPTION");
