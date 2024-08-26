# Compilation Options
CC = gcc
CC_FLAGS = -Wall \
	-pedantic \
	-g \
	-pthread \
	-o
C_SOURCE = $(wildcard *.c)
C_FINAL = $(basename ${C_SOURCE})


# Compile All .c files in the folder as their basename
all: ${C_FINAL}

% : %.c
	@ echo 'Compiling $< as $@...'
	@ $(CC) $< $(CC_FLAGS) $@
	@ echo 'Finished Building $@'


# Clean All Compiled Files, Auto Save and Core Files
clean: clean_obj clean_core clean_auto_save

clean_obj:
	@ echo "Cleaning All Object Files..."
	@ rm -f ${C_FINAL}

clean_core:
	@ echo "Cleaning All Core Files..."
	@ rm -f ./core*

clean_auto_save:
	@ echo "Cleaning All AutoSave Files.."
	@ rm -f *~
