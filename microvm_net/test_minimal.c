#include <stdio.h>
#include <stdlib.h>
#include "../include/microvm_net.h"

int main() {
    printf("Testing microvm_net initialization...\n");
    
    if (microvm_net_init() != 0) {
        printf("FAILED: microvm_net_init()\n");
        return 1;
    }
    printf("SUCCESS: microvm_net_init()\n");
    
    printf("Testing channel creation...\n");
    microvm_net_channel_t *channel = microvm_net_channel_create(NULL);
    if (!channel) {
        printf("FAILED: microvm_net_channel_create()\n");
        microvm_net_cleanup();
        return 1;
    }
    printf("SUCCESS: microvm_net_channel_create()\n");
    
    microvm_net_channel_destroy(channel);
    microvm_net_cleanup();
    
    printf("All tests passed!\n");
    return 0;
}
