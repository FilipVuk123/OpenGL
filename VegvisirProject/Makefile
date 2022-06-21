CC=gcc

CFLAGS   := -Iinclude  -pipe -Wall -pedantic -g -Wextra
LDFLAGS  := 
LDLIBS   := -lm -ldl -lglfw -lpthread -lavformat -lavcodec -lavutil 

SRC_DIR:=src
BIN_DIR:=bin
OBJ_DIR:=bin/obj

EXE:=$(BIN_DIR)/Vegvisir
SRC:=$(wildcard $(SRC_DIR)/*.c)
OBJ:=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))


.PHONY: all

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -r $(OBJ_DIR)


.PHONY: start

start:
	./$(EXE)