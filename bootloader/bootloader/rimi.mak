# WARNING!!! make sure you run this makefile from within a bash shell!
# If you run it from a Windows shell using "c:\cygwin\bin\make", the 
# windows binaries are before the CygWin binaries in the path, and all
# bets are off. Weirdo error messages to boot.
#
# All this include file does is to define the ECOS_* variables
# for stuff such as compilation options.
include $(INSTALL_DIR)/include/pkgconf/ecos.mak


# This is our output directory
OUTPUT = output

# This will enumerate all .cc files in the current directory and
# create a list of .o files we want to generate
CPPFILESLISTED := $(shell find "." -iname "*.cc")
CFILESLISTED := $(shell find "." -iname "*.c")
RAWDATAFILES := $(shell find "." -iname "*.rawdata")
ALIGNEDRAWDATAFILES := $(shell find "." -iname "*.alignedraw")

LINKFILES := $(patsubst %.alignedraw,$(OUTPUT)/%.o, $(ALIGNEDRAWDATAFILES))  $(patsubst %.rawdata,$(OUTPUT)/%.o, $(RAWDATAFILES)) $(patsubst %.rawdata,$(OUTPUT)/%_raw.o, $(RAWDATAFILES)) $(patsubst %.cc,$(OUTPUT)/%.o, $(CPPFILESLISTED)) $(patsubst %.c,$(OUTPUT)/%.o, $(CFILESLISTED))
# C++ compile setup, eCos specific.
XCC           = $(ECOS_COMMAND_PREFIX)gcc
XCXX          = $(ECOS_COMMAND_PREFIX)g++

XLD           = $(XCXX)

CFLAGS        = -I$(INSTALL_DIR)/include -I`pwd`

# Tricky! Here be dragons!
# enabling vtable garbage collection will cause a linker segmentation fault.
# optimisations are disabled for now to simplify debugging.
CFLAGS        += $(ECOS_GLOBAL_CFLAGS) 
CXXFLAGS	  =

CXXFLAGS      += -DPOSIX_EXCEPTIONS=1
CXXFLAGS      += -D__ECOS__=1 
CXXFLAGS      += $(CFLAGS) -O0 -fexceptions 
LDFLAGS       =  $(ECOS_GLOBAL_LDFLAGS)  -nostartfiles -L$(INSTALL_DIR)/lib -Ttarget.ld   -Wl,--undefined=pthread_mutex_lock



# these are not files that should be date checked
.PHONY : clean all $(OUTPUT) 

ALLDEP = $(OUTPUT)/$(APPNAME).bin 
 
all: $(ALLDEP) 

$(OUTPUT):
	mkdir --parents $(OUTPUT)

clean: 
	echo Cleaning
	rm -rf $(OUTPUT)

# This tells GCC the form that the dependency rules should have.
GENOPT = -MP -MT '$(OUTPUT)/$*.o' -MT '$(OUTPUT)/$*.d'

.SILENT: $(LINKFILES) $(LINKFILES:.o=.d) $(OUTPUT)/$(APPNAME).bin $(OUTPUT)/version.txt 

# some singing and dancing to get an 4 bytes aligned data. 
# raw data
$(OUTPUT)/%.o: %.alignedraw
	mkdir --parents $(@D)
	echo -- Raw data file $*.alignedraw
	../scripts/myfile2c.tcl $*.alignedraw $(OUTPUT)/$*.txt
	echo >$(OUTPUT)/$*.xxxx "const __attribute__((aligned(4))) char "
	echo $(OUTPUT)/$* | sed -e 's/\//_/g'  | sed -e 's/\./_/g' >>$(OUTPUT)/$*.xxxx
	echo []= >>$(OUTPUT)/$*.xxxx
	cat $(OUTPUT)/$*.txt >>$(OUTPUT)/$*.xxxx
	$(XCC) -o $(OUTPUT)/$*.o $(CFLAGS) -c -x c $(OUTPUT)/$*.xxxx

# convert raw data to .o files.
$(OUTPUT)/%.o: %.rawdata
	mkdir --parents $(@D)
	echo -- Raw data file $*.rawdata
	# Here be dragons!!! workaround for problems with -c option in gzip.
	cp $*.rawdata $(OUTPUT)/$*
	gzip -f -9 $(OUTPUT)/$*
	nios2-elf-objcopy -B arm -I binary -O elf32-littlearm --rename-section .data=.rodata $*.rawdata $(OUTPUT)/$*_raw.o
	nios2-elf-objcopy -B arm -I binary -O elf32-littlearm --rename-section .data=.rodata $(OUTPUT)/$*.gz $(OUTPUT)/$*.o
#	objcopy -B i386 -I binary -O elf32-i386  --rename-section .data=.rodata $*.rawdata $(OUTPUT)/$*_raw.o
#	objcopy -B i386 -I binary -O elf32-i386  --rename-section .data=.rodata $(OUTPUT)/$*.gz $(OUTPUT)/$*.o

# Here we compile .cc to .o and .d files at the same time
$(OUTPUT)/%.o: %.cc
	mkdir --parents $(@D)
	echo -- compiling $*.cc
	$(XCXX) $(GENOPT) -MD -c -o $(OUTPUT)/$*.o $(CXXFLAGS) $<


$(OUTPUT)/%.o: %.c
	mkdir --parents $(@D)
	echo -- compiling $*.c
	$(XCC) $(GENOPT) -MD -c -o $(OUTPUT)/$*.o $(CFLAGS) $<


$(OUTPUT)/%.bin: 
	echo -- linking 
	# we don't rebuild version number unless something else changed
	echo "-- updating build date(version.cc)"
	$(XCXX) $(GENOPT) -MD  -c -o $(OUTPUT)/version.o $(CXXFLAGS) version.cc 
	$(XLD) $(LDFLAGS)  -o $(OUTPUT)/$*.elf $^  $(LDAFTEROBJ) -Wl,-Map,$(OUTPUT)/$*.map   -lstdc++
	nios2-elf-objcopy -O srec $(OUTPUT)/$*.elf $(OUTPUT)/$*.srec
	nios2-elf-objcopy -O binary $(OUTPUT)/$*.elf $(OUTPUT)/$*.bin
	#kludge!!! workaround for problems in gzip stdout handlings
	gzip -f -9 $(OUTPUT)/$*.bin
	nios2-elf-objcopy -O binary $(OUTPUT)/$*.elf $(OUTPUT)/$*.bin
	nios2-elf-size $(OUTPUT)/$(APPNAME).elf

# Tricky!
# if a .d file does not exist, it will be compiled
-include $(LINKFILES:.o=.d)

$(OUTPUT)/$(APPNAME).bin: $(LINKFILES) 