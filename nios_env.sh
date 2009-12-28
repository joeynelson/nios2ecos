# set up enviornment variables. 
#
# cd nios2ecos
# source nios_env.sh
#
# Intended to work on Cygwin & Linux out of the box(tested on Ubuntu 9.04,
# 9.10 and Cygwin as of writing).

# Be afraid!!!! here we have to figure out the location of the
# script being sourced!!!! Requires bash 3.0 or later.
export NIOS2_ENV_PATH=$(dirname $BASH_ARGV)
export NIOS_ECOS=$(readlink -f $NIOS2_ENV_PATH/packages)

echo "Test $BASH_ARGV"
echo "NIOS_ECOS = $NIOS_ECOS"

if [ `uname` = Linux ] ;then
	echo "Linux box"
	if [ -d /opt/altera9.1 ]; then
	    export CYG_ALTERA_ROOTDIR=/opt/altera9.1
	else
	    export CYG_ALTERA_ROOTDIR=/opt/altera9.0
	fi
	# FIX!!!! check if nios2-elf-gcc is already added to path
	export PATH=$PATH:$CYG_ALTERA_ROOTDIR/nios2eds/bin/nios2-gnutools/H-i686-pc-linux-gnu/bin
	export TMP=/tmp
	# SOPC_KIT_NIOS2 and QUARTUS_ROOTDIR exists in the 
	# Windows environment when Quartus 9 is installed
	# and there are scripts that rely on their presence. Add
	# them for Linux.
	export SOPC_KIT_NIOS2=$CYG_ALTERA_ROOTDIR/nios2eds
	export QUARTUS_ROOTDIR=$CYG_ALTERA_ROOTDIR/quartus
else
	echo "Cygwin"
	export CYGWIN=nontsec
	# DANGER!!! here we need windows-like paths for compatibility.
	export WIN_ALTERA_ROOTDIR=`cygpath -m $SOPC_KIT_NIOS2/.. | sed "s,/$,,"`
	export CYG_ALTERA_ROOTDIR=`cygpath -u $WIN_ALTERA_ROOTDIR`
	# FIX!!!! check if nios2-elf-gcc is already added to path
	export PATH=$PATH:$CYG_ALTERA_ROOTDIR/nios2eds/bin/nios2-gnutools/H-i686-pc-cygwin/bin
fi


# This is the standard installation directory for eCos 3.0
# echo "Adding eCos 3.0 tools to path: ~/ecos/ecos-3.0/tools/bin"
# FIX!!! check if ecosconfig is already in the path and only add
# this to path if it isn't 
# export PATH=$PATH:~/ecos/ecos-3.0/tools/bin
# FIX!!! ECOS_REPOSITORY must already be set up, add check
#export ECOS_REPOSITORY=~/ecos/ecos-3.0/packages
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

# These two last paths are not necessary to build stuff, only to
# get tools to communicate with the FPGA(nios2-gdb-server, etc.)

export PATH=$PATH:$CYG_ALTERA_ROOTDIR/nios2eds/bin
export PATH=$PATH:$QUARTUS_ROOTDIR/bin

# Workaround for cygpath problems.
if [ `uname` = Linux ] ;then
	export PATH=$PATH:$NIOS_ECOS/hal/nios2/arch/current/host/cygpath
fi
