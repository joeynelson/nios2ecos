# set up enviornment variables. 
#
# cd nios2ecos
# source nios_env.sh
#
# Intended to work on Cygwin & Linux out of the box(tested on Ubuntu 9.04
# as of writing).
set -e
export CYGWIN=nontsec
export NIOS_ECOS=`pwd`/packages


if [ `uname` = Linux ] ;then
	echo "Linux box"
	export WIN_ALTERA_ROOTDIR=/opt/altera9.0/
	export CYG_ALTERA_ROOTDIR=/opt/altera9.0/
	export PATH=$PATH:$CYG_ALTERA_ROOTDIR/nios2eds/bin/nios2-gnutools/H-i686-pc-linux-gnu/bin
	export TMP=/tmp
else
	echo "Cygwin"
	export WIN_ALTERA_ROOTDIR=c:/altera/90
	export CYG_ALTERA_ROOTDIR=/cygdrive/c/altera/90
	export PATH=$PATH:$CYG_ALTERA_ROOTDIR/nios2eds/bin/nios2-gnutools/H-i686-pc-cygwin/bin
fi


# DANGER!!! here we need windows-like paths for compatibility.
export QUARTUS_ROOTDIR=$WIN_ALTERA_ROOTDIR/quartus
export SOPC_KIT_NIOS2=$WIN_ALTERA_ROOTDIR/nios2eds
export PERL5LIB=/bin:$CYG_ALTERA_ROOTDIR/quartus/sopc_builder/bin/perl_lib:$CYG_ALTERA_ROOTDIR/quartus/sopc_builder/bin/europa:$CYG_ALTERA_ROOTDIR/quartus/sopc_builder/bin


# generally place the altera stuff *LAST* in the path because it contains
# lots of obsolete stuff
export PATH=$PATH:$NIOS_ECOS/hal/nios2/arch/current/host
export PATH=$PATH:$CYG_ALTERA_ROOTDIR/quartus/sopc_builder/bin
export PATH=$PATH:$CYG_ALTERA_ROOTDIR/quartus/bin
export PATH=$PATH:$CYG_ALTERA_ROOTDIR/nios2eds/bin

#export PATH=$CYG_ALTERA_ROOTDIR/nios2eds/bin:$CYG_ALTERA_ROOTDIR/nios2eds/sdk2/bin:$CYG_ALTERA_ROOTDIR/nios2eds/bin/fs2/bin:$CYG_ALTERA_ROOTDIR/quartus/win:$CYG_ALTERA_ROOTDIR/quartus/bin:$CYG_ALTERA_ROOTDIR/quartus/bin/perl/bin:$CYG_ALTERA_ROOTDIR/quartus/bin/gnu:$PATH



export ECOS_REPOSITORY=$NIOS_ECOS:`pwd`/../ecos/packages


if [ `uname` = Linux ] ;then
	# Workaround for cygpath problems.
	export PATH=$PATH:$NIOS_ECOS/hal/nios2/arch/current/host/cygpath	
fi
