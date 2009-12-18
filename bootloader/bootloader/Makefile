PROJECTNAME = bootloader
OUT = out
PROJECTNAME = bootloader
INSTALL_DIR=${ECOS_INSTALL}/install
include $(INSTALL_DIR)/include/pkgconf/ecos.mak

XCC           = $(ECOS_COMMAND_PREFIX)g++
XCXX          = $(XCC)
XLD           = $(XCC)
OBJCOPY		  = $(ECOS_COMMAND_PREFIX)objcopy
OBJDUMP		  = $(ECOS_COMMAND_PREFIX)objdump

CFLAGS        = -I$(INSTALL_DIR)/include -fexceptions -O0 -Wall -Wformat-security -Wextra -Wno-unused-parameter -Wcast-align 
CXXFLAGS      = $(CFLAGS)
LDFLAGS       = -nostartfiles  -lstdc++ -L$(INSTALL_DIR)/lib -Ttarget.ld -Wl,-Map,$(OUT)/$(PROJECTNAME).map -N

FILES		=  ${OUT}/bootloader.o  ${OUT}/xyzModem.o



all: $(OUT) ${OUT}/$(PROJECTNAME).bin transform

$(OUT):
	mkdir $(OUT)

clean:
	rm -rf ${OUT}

transform:
	elf2flash --input=${OUT}/bootloader.elf --output=${OUT}/bootloader.flash --base=0x4000000 --end=0x5FFFFFF --reset=0x4120000 --boot=$(SOPC_KIT_NIOS2)/components/altera_nios2/boot_loader_cfi.srec
	./bin2srec/out/srec2bin ${OUT}/bootloader.flash ${OUT}/bootloader.bin -o 0x4120000

${OUT}/%.o: %.c*
	$(XCC) -c -o ${OUT}/$*.o $(ECOS_GLOBAL_CFLAGS) $(CFLAGS) `pwd`/$<
	
${OUT}/$(PROJECTNAME).elf: $(FILES)
	$(XLD) $(FILES) $(LDFLAGS) $(ECOS_GLOBAL_LDFLAGS)  -o $@ 

${OUT}/$(PROJECTNAME).bin: ${OUT}/$(PROJECTNAME).elf	
	$(OBJCOPY) -O binary $< $@ 
	$(OBJCOPY) -O srec --srec-forceS3 $< ${OUT}/$(PROJECTNAME).flash	
	$(OBJDUMP) -d $<  > ${OUT}/$(PROJECTNAME).asm