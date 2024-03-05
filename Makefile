SRC_DIR := ./src
UTIL_DIR := ./util
BUILD_DIR := ./build

CC := gcc
OPT_LEVEL ?= Og
CFLAGS := -Wall -Wno-unused-result -Wno-incompatible-pointer-types -$(OPT_LEVEL) -I $(SRC_DIR) -I $(UTIL_DIR)

SRC := $(wildcard $(SRC_DIR)/*.c)
UTIL_SRC := $(wildcard $(UTIL_DIR)/*.c)

OBJ := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))
UTIL_OBJ := $(patsubst $(UTIL_DIR)/%.c, $(BUILD_DIR)/%.o, $(UTIL_SRC))
DEP := $(OBJ:.o=.d) $(UTIL_OBJ:.o=.d)

TARGET := ./mainProg

all: $(TARGET)
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
rebuild: clean all
-include $(DEP)

# 生成目标文件
$(TARGET): $(OBJ) $(UTIL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MF $(@:.o=.d)
$(BUILD_DIR)/%.o: $(UTIL_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MF $(@:.o=.d)
