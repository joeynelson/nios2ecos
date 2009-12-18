# Makefile

CFLAGS=-c -Wall
LDFLAGS=-fno-exceptions -s

MAKEFLAGS += -s

ifeq ($(OS), Windows_NT)
	CC = mingw32-gcc
	EXE_SUFFIX = .exe
else
	CC = gcc
endif

BIN2SREC_EXE = ${OUT}/bin2srec$(EXE_SUFFIX)
SREC2BIN_EXE = ${OUT}/srec2bin$(EXE_SUFFIX)
OUT = out

all: $(OUT) $(BIN2SREC_EXE) $(SREC2BIN_EXE)	

BIN2SREC_SRC = \
	bin2srec.c \
	common.c \

SREC2BIN_SRC = \
	srec2bin.c \
	common.c \

BIN2SREC_OBJS = ${OUT}/$(BIN2SREC_SRC:.c=.o)
SREC2BIN_OBJS = ${OUT}/$(SREC2BIN_SRC:.c=.o)	

$(OUT):
	mkdir $(OUT)

${OUT}/%.o:%.c	Makefile
	@echo CC $<
	@echo $(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN2SREC_EXE): $(BIN2SREC_OBJS)
	@echo Linking $@
	$(CC) $(LDFLAGS) $^ -o $@

$(SREC2BIN_EXE): $(SREC2BIN_OBJS)
	@echo Linking $@
	$(CC) $(LDFLAGS) $^ -o $@


clean:
	-$(RM) $(BIN2SREC_OBJS) $(SREC2BIN_OBJS)
	-$(RM) $(BIN2SREC_EXE) $(SREC2BIN_EXE)
