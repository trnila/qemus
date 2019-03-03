#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main() {
  int fd = open("/dev/hello", O_RDWR);
  if(fd < 0) {
    perror("failed to open");
    return -1;
  }

  write(fd, "ahoj!", 5);

/*  ioctl(fd, 42, 43);
  open("/etc/shadow", O_RDWR);
  execl("/bin/bash", "/bin/bash", NULL);
  */
}
