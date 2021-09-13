CC=gcc

CFLAGS   := -Iinclude  -pipe -Wall -Wextra -pedantic -g
LDFLAGS  := -ldl -lpthread -lavformat -lavcodec -lswscale -lz -lavutil 
LDLIBS   := -lm -ldl -lglfw -lpthread -lavformat -lavcodec -lswscale -lz -lavutil


SRC_DIR:=src
BIN_DIR:=bin
OBJ_DIR:=lib

EXE:=$(BIN_DIR)/SphereProject
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
