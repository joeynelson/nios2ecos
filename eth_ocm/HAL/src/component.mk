
C_LIB_SRCS += ins_eth_ocm.c eth_ocm_phy.c
ASM_LIB_SRCS +=
INCLUDE_PATH +=                                                                \

# We define the ALTERA_TRIPLE_SPEED_MAC compiler flag because
# currently, Altera's implementation of the InterNiche ipport.h
# file forces the stack to use a different function for allocating
# buffers which bypasses the NIOS II processors cache. We want
# to do this as well so we don't have to copy
CPPFLAGS += -DALTERA_TRIPLE_SPEED_MAC
