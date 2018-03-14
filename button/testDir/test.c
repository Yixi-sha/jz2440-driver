#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

void my_signal_fun(int signal)
{
    printf("signal %d\n",signal);
}

int main(int argc,char** argv)
{
    int fd = 0;
    int val = 0;
    unsigned char key_val[4];
    int ret;
    struct pollfd pFd;
    int Oflags = 0;
    signal(SIGIO,my_signal_fun);

    printf("open this file \n");
    fd = open("/dev/myButton", O_RDWR );
    if(fd < 0)
    {
        printf("open this file fail\n");
        return 0;
    }
    fcntl(fd, F_SETOWN,getpid());
    Oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, Oflags| FASYNC);
    pFd.fd = fd;
    pFd.events = POLLIN;
    /*while(1)
    {
        ret =  poll(&pFd, 1, 5000);
        if(ret == 0)
        {
            printf("time out\n");
        }
        else
        {
            ret = read(fd, key_val, sizeof(key_val));
            if( (!key_val[0]) || (!key_val[1]) || (!key_val[2]) || (!key_val[3]))
            {
                printf("key_val is %d %d %d %d\n", key_val[0], key_val[1], key_val[2], key_val[3]);
            }
        }
        
        
    }*/
    sleep(5);
    close(fd);
    


   
    return 0;
}
