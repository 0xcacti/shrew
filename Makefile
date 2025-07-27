# === Config ===
TARGET     := bin/shrew
TEST_BIN   := bin/tests

SRC_DIR    := src
OBJ_DIR    := obj
BIN_DIR    := bin
INCLUDE    := include
TEST_DIR   := tests

SRC        := $(wildcard $(SRC_DIR)/*.c)
OBJ        := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
LIB_OBJ    := $(filter-out $(OBJ_DIR)/main.o, $(OBJ))

TEST_SRC   := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJ   := $(TEST_SRC:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.test.o)

PKG_CFLAGS := $(shell pkg-config --cflags criterion)
PKG_LIBS   := $(shell pkg-config --libs   criterion)

CFLAGS     := -Wall -Wextra -O2 -I$(INCLUDE) $(PKG_CFLAGS)
LDLIBS     := $(PKG_LIBS)

# === Default Build ===
all: $(TARGET)

# === Main Target ===
$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# === Object Files from src/ ===
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# === Object Files from tests/ ===
$(OBJ_DIR)/%.test.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# === Test Target ===
$(TEST_BIN): $(LIB_OBJ) $(TEST_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

test: $(TEST_BIN)
	./$(TEST_BIN)

# === Directories ===
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# === Clean Build ===
clean:
	rm -rf $(OBJ_DIR)/*.o $(OBJ_DIR)/*.test.o $(TARGET) $(TEST_BIN)

# === Compile Commands for LSPs ===
cdb:
	@rm -f compile_commands.json
	@compiledb --output compile_commands.json make clean all
	@echo "✓ compile_commands.json regenerated"

.PHONY: all clean test cdb

