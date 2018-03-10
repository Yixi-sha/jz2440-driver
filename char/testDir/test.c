#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char** argv)
{
    int fd = 0;
    int val = 0;
    fd = open("/dev/myLed", O_RDWR);
    if(fd < 0)
    {
        printf("can't open this file \n");
    }
    if(argc != 2)
    {
        printf("plase chose <on or off> \n");
    }
    else
    {
        if(fd > 0 )
        {
            if(strcmp(argv[1],"on") == 0)
            {
                val = 1;
            }
            else
            {
                val = 0;
            }
            write(fd,&val,4);
            
            
        }
        
    }
    if(fd > 0)
    {
        close(fd);
    }
   
    return 0;
}
