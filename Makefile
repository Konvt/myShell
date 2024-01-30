CC = gcc
CFLAGS = -Og -Wall

PACK_PATH = ./
PACK_NAME = $(shell basename $(shell pwd))

TARGET = mainProg
OJB_DIR = ./build
LIB_DIR = ./include
SRC_DIR = $(dir ./src/mainProg.c)

GCC_SOURCE_PATH = $(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))
GCC_OBJ_PATH = $(patsubst %.c, $(OJB_DIR)/%.o, $(notdir $(GCC_SOURCE_PATH)))

$(TARGET): $(GCC_OBJ_PATH)
	$(CC) $(GCC_OBJ_PATH) -I $(LIB_DIR) $(CFLAGS) -o $(TARGET)

-include $(OJB_DIR)/*.d
$(OJB_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -MMD -I $(LIB_DIR) $(LIB_PATH) $< -o $@

# clean up all intermediate files
.PHONY: clean
clean:
	rm $(GCC_OBJ_PATH) $(TARGET) && make independ

# only clean up .d files
.PHONY: independ
independ:
	rm $(OJB_DIR)/*.d

# package the source code into a .tar file
.PHONY: pack
pack:
	tar czvf $(PACK_PATH)/$(PACK_NAME).tar ./

# create the project dirs and files
.PHONY: new
new:
	mkdir src include build && touch ./src/mainProg.c README.md
