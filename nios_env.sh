# set up enviornment variables. 
#
# cd nios2ecos
# source nios_env.sh
#
# Intended to work on Cygwin & Linux out of the box(tested on Ubuntu 9.04
# as of writing).

export NIOS_ECOS=`pwd`/packages

if [ `uname` = Linux ] ;then
	echo "Linux box"
	export WIN_ALTERA_ROOTDIR=/opt/altera9.0
	export CYG_ALTERA_ROOTDIR=/opt/altera9.0
	# FIX!!!! check if nios2-elf-gcc is already added to path
	export PATH=$PATH:$CYG_ALTERA_ROOTDIR/nios2eds/bin/nios2-gnutools/H-i686-pc-linux-gnu/bin
	export TMP=/tmp
else
	echo "Cygwin"
	export CYGWIN=nontsec
	export WIN_ALTERA_ROOTDIR=c:/altera/90
	export CYG_ALTERA_ROOTDIR=/cygdrive/c/altera/90
	# FIX!!!! check if nios2-elf-gcc is already added to path
	export PATH=$PATH:$CYG_ALTERA_ROOTDIR/nios2eds/bin/nios2-gnutools/H-i686-pc-cygwin/bin
fi

# DANGER!!! here we need windows-like paths for compatibility.
export QUARTUS_ROOTDIR=$WIN_ALTERA_ROOTDIR/quartus
export SOPC_KIT_NIOS2=$WIN_ALTERA_ROOTDIR/nios2eds
export PERL5LIB=/bin:$CYG_ALTERA_ROOTDIR/quartus/sopc_builder/bin/perl_lib:$CYG_ALTERA_ROOTDIR/quartus/sopc_builder/bin/europa:$CYG_ALTERA_ROOTDIR/quartus/sopc_builder/bin

# This is the standard installation directory for eCos 3.0
echo "Adding eCos 3.0 tools to path: ~/ecos/ecos-3.0/tools/bin"
# FIX!!! check if ecosconfig is already in the path and only add
# this to path if it isn't 
export PATH=$PATH:~/ecos/ecos-3.0/tools/bin
# FIX!!! check if ECOS_REPOSITORY variable is already set up
export ECOS_REPOSITORY=~/ecos/ecos-3.0/packages
echo "Prepend Nios eCos repository to ECOS_REPOSITORY=$ECOS_REPOSITORY"
export ECOS_REPOSITORY=$NIOS_ECOS:$ECOS_REPOSITORY

# Enable the line below if you need to build libstdc++ posix threads 
# This is the compat/posix support while we wait for a few more features
# to be added to the eCos CVS HEAD
#export ECOS_REPOSITORY=$NIOS_ECOS/../tools/gcc4libstdxx/ecos:$ECOS_REPOSITORY
echo "ECOS_REPOSITORY=$ECOS_REPOSITORY"


# generally place the altera stuff *LAST* in the path because it contains
# lots of obsolete stuff
export PATH=$PATH:$NIOS_ECOS/hal/nios2/arch/current/host
export PATH=$PATH:$CYG_ALTERA_ROOTDIR/quartus/sopc_builder/bin



# Workaround for cygpath problems.
if [ `uname` = Linux ] ;then
	export PATH=$PATH:$NIOS_ECOS/hal/nios2/arch/current/host/cygpath
fi
