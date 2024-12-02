# Compilation Options
CC = gcc
CC_FLAGS = -Wall \
	-pedantic \
	-lm \
	-I $(INCLUDE) \
	-g \
	-O2 \
	-o

# Folders
SRC = ./src
INCLUDE = ./include
C_SOURCE = $(wildcard ${SRC}/*.c)
C_FILES = $(subst ${SRC}/,,${C_SOURCE})
C_FINAL = $(basename $(C_FILES))


# Compile All .c files in the folder as their basename
all: ${C_FINAL}

% : ${SRC}/%.c
	@ echo 'Compiling $< as $@...'
	@ $(CC) $< $(CC_FLAGS) $@
	@ echo '$@ Compiled!'


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
	@ rm -rf ./*~
