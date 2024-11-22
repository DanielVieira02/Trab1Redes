#include "kermit.h"

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("Usage: ./main <IS_CLIENT> \n");
        return -1;
    }

    int isClient = (atoi(argv[1]) == 1);
    
    if(isClient) {
        client();
    } else {
        server();
    }

    return 0;
}