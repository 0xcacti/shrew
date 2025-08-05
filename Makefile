# === Paths & Targets ===
SRC_DIR        := src
OBJ_DIR        := obj
BIN_DIR        := bin
INCLUDE        := include
TEST_DIR       := tests

TARGET         := $(BIN_DIR)/shrew
TEST_BIN       := $(BIN_DIR)/tests

ASAN_SUFFIX    := .asan
ASAN_TARGET    := $(TARGET)$(ASAN_SUFFIX)
ASAN_TEST_BIN  := $(TEST_BIN)$(ASAN_SUFFIX)

# === Source & Object Files ===
SRC            := $(wildcard $(SRC_DIR)/*.c)
OBJ            := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
LIB_OBJ        := $(filter-out $(OBJ_DIR)/main.o, $(OBJ))

TEST_SRC       := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJ       := $(TEST_SRC:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.test.o)

ASAN_OBJ       := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.asan.o)
ASAN_LIB_OBJ   := $(filter-out $(OBJ_DIR)/main.asan.o, $(ASAN_OBJ))
ASAN_TEST_OBJ  := $(TEST_SRC:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.test.asan.o)

# === Build Flags ===
PKG_CFLAGS     := $(shell pkg-config --cflags criterion)
PKG_LIBS       := $(shell pkg-config --libs criterion)

CFLAGS         := -Wall -Wextra -O2 -I$(INCLUDE) $(PKG_CFLAGS)
LDLIBS         := $(PKG_LIBS)

ASAN_CFLAGS    := -fsanitize=address -g -O1

# === Build Targets ===
all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_BIN): $(LIB_OBJ) $(TEST_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(ASAN_TARGET): $(ASAN_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $^

$(ASAN_TEST_BIN): $(ASAN_LIB_OBJ) $(ASAN_TEST_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -o $@ $^ $(LDLIBS)

# === Object File Rules ===
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.test.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.asan.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.test.asan.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(ASAN_CFLAGS) -c $< -o $@

# === Phony Run Targets ===
test: $(TEST_BIN)
	@./$(TEST_BIN)

asan-main: $(ASAN_TARGET)
	@echo "✓ Built $(ASAN_TARGET) with ASAN"

asan-test: $(ASAN_TEST_BIN)
	@echo "=== Running tests with ASAN ==="
	@MallocNanoZone=0 ./$(ASAN_TEST_BIN)

asan: asan-main asan-test

# === Directories ===
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# === Clean Targets ===
clean:
	rm -rf $(OBJ_DIR)/*.o $(OBJ_DIR)/*.test.o $(TARGET) $(TEST_BIN)

clean-asan:
	rm -rf $(OBJ_DIR)/*.asan.o $(OBJ_DIR)/*.test.asan.o $(ASAN_TARGET) $(ASAN_TEST_BIN)

# === LSP Support ===
cdb:
	@rm -f compile_commands.json
	@compiledb --output compile_commands.json make clean all
	@echo "✓ compile_commands.json regenerated"

.PHONY: all clean clean-asan test cdb asan asan-main asan-test

