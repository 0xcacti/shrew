# === Config ===
TARGET=bin/shrew
SRC=$(wildcard src/*.c)
OBJ=$(SRC:src/%.c=obj/%.o)
CFLAGS=-Wall -Wextra -O2 -Iinclude

# === Default Rule ===
$(TARGET): $(OBJ) | bin
	$(CC) $(CFLAGS) -o $@ $^

# === Object Files Rule ===
obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

# === Directories Rules ===
bin:
	mkdir -p bin

obj:
	mkdir -p obj


# === Cleaning Rule ===
clean: 
	rm -f obj/*.o $(TARGET)

# === Compile Commands Rule ===
cdb:
	@rm -f compile_commands.json
	@compiledb --output compile_commands.json make clean default
	@echo "✓ compile_commands.json regenerated"

.PHONY: clean
