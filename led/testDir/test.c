#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char** argv)
{
    int fd = 0;
    int val = 0;
    printf("open this file \n");
    if(argc != 3)
    {
        printf("plase chose <ptah> <on or off> \n");
    }
    else
    {
        fd = open(argv[1], O_RDWR);
        if(fd < 0)
        {
            printf("can't open this file \n");
        }
        else
        {
            if(strcmp(argv[2],"on") == 0)
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
