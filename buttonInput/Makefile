.PHONY :clean
KERN_DIR = /home/yixi/work/study/JZ2440/linux-3.4.2

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= buttonInput.o
