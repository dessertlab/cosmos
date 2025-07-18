# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -g -D_DBG_ -D_DBG_TEST_ -pthread -fsanitize=address

# Include paths
INCLUDES := -I.. -I../injector

# Source files and executable name
SRCS := cosmos_nii_inj.c ../generic_ops.c ../injector/operation_lib.c ../injector/definitions.c 
EXE := cosmos_nii_inj

all: $(EXE)

$(EXE): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LDLIBS) -o $@

clean:
	rm -f $(EXE)

.PHONY: clean
