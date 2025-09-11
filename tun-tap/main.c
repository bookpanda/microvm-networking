#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

int tun_alloc(char *dev) {
    struct ifreq ifr;
    int fd = open("/dev/net/tun", O_RDWR); // O_RDWR = r/w packets
    if (fd < 0) {
        perror("Opening /dev/net/tun");
        exit(1);
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;  // Create TUN (no Ethernet header)

    // If the caller provided a name (tun0), copy it
    if (*dev)
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    // Ask the kernel driver to create a TUN device with these flags.
    if (ioctl(fd, TUNSETIFF, (void *)&ifr) < 0) {
        // if the program doesn’t have root privileges
        perror("ioctl(TUNSETIFF)");
        close(fd);
        exit(1);
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}

int main() {
    char tun_name[IFNAMSIZ] = "tun0";
    int fd = tun_alloc(tun_name);

    printf("TUN interface %s created\n", tun_name);

    char buffer[1500];
    while (1) {
        int nread = read(fd, buffer, sizeof(buffer));
        if (nread > 0) {
            printf("Read %d bytes from %s\n", nread, tun_name);
        }
    }
}
