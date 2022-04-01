CC=gcc

CFLAGS   := -Iinclude -pipe -Wall -Wextra -g
LDFLAGS  := 
LDLIBS   := -lm -ldl -lglfw

SRC_DIR:=src
BIN_DIR:=bin
OBJ_DIR:=lib

EXE:=$(BIN_DIR)/RawImageProject
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))


.PHONY: all

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -r $(OBJ_DIR)