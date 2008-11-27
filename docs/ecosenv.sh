# Warning! This is a generated file used by the eCos installer.
# If you edit this file, you may break future upgrades using the installer.

#ECOS_REPOSITORY=`cygpath -u $SOPC_KIT_NIOS2`/components/ecos/ecos-current/packages ; export ECOS_REPOSITORY

# eCos paths - do not modify this line, it is used by the installer
#PATH=`cygpath -u $SOPC_KIT_NIOS2`/components/ecos/ecos-2.0/tools/bin:$PATH ; export PATH
# End eCos paths - do not modify this line, it is used by the installer

ECOS_REPOSITORY=`cygpath -ma ecos-current/packages` ; export ECOS_REPOSITORY
PATH=`pwd`/ecos-current/tools/bin:`pwd`/ecos-current/packages/hal/nios2/arch/current/host:$PATH ; export PATH
