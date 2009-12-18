PROJECTNAME = deflator
OUT = out
PROJECTNAME = deflator
INSTALL_DIR=${ECOS_INSTALL}/install
include $(INSTALL_DIR)/include/pkgconf/ecos.mak

XCC           = $(ECOS_COMMAND_PREFIX)gcc
XCXX          = $(XCC)
XLD           = $(XCC)
OBJCOPY		  = $(ECOS_COMMAND_PREFIX)objcopy
OBJDUMP		  = $(ECOS_COMMAND_PREFIX)objdump

CFLAGS        = -I$(INSTALL_DIR)/include -fexceptions -O0
CXXFLAGS      = $(CFLAGS)
LDFLAGS       = -nostartfiles  -lstdc++ -L$(INSTALL_DIR)/lib -Ttarget.ld -Wl,-Map,$(OUT)/$(PROJECTNAME).map -N

FILES		=  ${OUT}/deflator.o

all: $(OUT) ${OUT}/$(PROJECTNAME).bin

$(OUT):
	mkdir $(OUT)

clean:
	rm -rf ${OUT}

#${OUT}/%.o: %.c
#	$(XCC) -c -o ${OUT}/$*.o $(CFLAGS) $(ECOS_GLOBAL_CFLAGS) $<

${OUT}/%.o: %.c*
	$(XCC) -c -o ${OUT}/$*.o $(ECOS_GLOBAL_CFLAGS) $(CFLAGS) $<
	
${OUT}/$(PROJECTNAME).elf: $(FILES)
	$(XLD) $(FILES) $(LDFLAGS) $(ECOS_GLOBAL_LDFLAGS)  -o $@ 

${OUT}/$(PROJECTNAME).bin: ${OUT}/$(PROJECTNAME).elf
	$(OBJCOPY) -O binary $< $@ 
	$(OBJCOPY) -O srec --srec-forceS3 --change-addresses -0x04000000 $< ${OUT}/$(PROJECTNAME).flash
	$(OBJDUMP) -d $<  > ${OUT}/$(PROJECTNAME).asm