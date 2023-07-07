#### Frugal GUI ####
ARMGNU ?= arm-none-eabi
COPS = -std=gnu99 -Wall -O2 -nostdlib -nostartfiles -ffreestanding -fdiagnostics-color

gui : kernel.img

clean :
	rm -f *.o
	rm -f *.bin
	rm -f *.hex
	rm -f *.elf
	rm -f *.list
	rm -f *.img





vectors.o : vectors.s
	$(ARMGNU)-as vectors.s -o vectors.o

util.o : util.c
	$(ARMGNU)-gcc $(COPS) -c util.c -o util.o

main.o : main.c
	$(ARMGNU)-gcc $(COPS) -c main.c -o main.o

#It seems the order of these is important. vectors must be first!
kernel.img : loader vectors.o util.o main.o 

	$(ARMGNU)-ld  vectors.o util.o  main.o    -T loader -o main.elf

	$(ARMGNU)-objdump -D main.elf > main.list
	$(ARMGNU)-objcopy main.elf -O ihex main.hex

	#convenient copy at top of transfer file list	
	$(ARMGNU)-objcopy main.elf -O ihex aaa.hex

	$(ARMGNU)-objcopy main.elf -O binary kernel.img

#### Pitaya Server ####

serv:
	gcc pitaya_serv.c -O3 -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -lpthread -lm  -lliquid -Wall -o serv

clean_serv:
	rm -f *.o serv
