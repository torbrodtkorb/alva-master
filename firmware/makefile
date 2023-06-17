PROGRAMMER = /mnt/c/Users/tor/AppData/Local/Programs/Python/Python311/Scripts/pymcuprog.exe
FLAGS      = -Wall -O1 -g3 -Wno-unused-function -Wno-unused-variable -DF_CPU=24000000UL -mmcu=avr64db32 -Bpack/gcc/dev/avr64db32 -Ipack/include -I.

.PHONY: build clean

build:
	@mkdir -p build
	@avr-gcc $(FLAGS) $^ *.c -o build/program.elf
	@avr-objcopy -j .text -j .data -j .bss -O ihex build/program.elf build/program.hex
	@avr-objdump -h -S build/program.elf > build/program.lss
	@sudo $(PROGRAMMER) erase -d avr64db32 -i updi
	@sudo $(PROGRAMMER) write -d avr64db32 -i updi --verify -f build/program.hex

clean:
	@rm -rf build
