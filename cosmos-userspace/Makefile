# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -g -D_DBG_ -D_DBG_TEST_ -pthread -fsanitize=address
LDLIBS := -pthread

# Include paths
INCLUDES := -I. -I./injector

# Source files and executable name
SRCS := cosmos_ui.c generic_ops.c injector/operation_lib.c injector/definitions.c 
EXE := cosmos_ui

all: $(EXE) 

$(EXE): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LDLIBS) -o $@

clean:
	rm -f $(EXE)

.PHONY: clean
