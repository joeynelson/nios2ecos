include ${ECOS_INSTALL}/install/include/pkgconf/ecos.mak

all:
	nios2-elf-gcc -lsupc++  $(ECOS_GLOBAL_LDFLAGS)  $(ECOS_GLOBAL_CFLAGS) -I$(ECOS_INSTALL)/install/include  -lstdc++ -L$(ECOS_INSTALL)/install/lib -Ttarget.ld test.cc -fexceptions -o test.elf
