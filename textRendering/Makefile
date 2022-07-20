CXX=g++
CC=gcc

CFLAGS   := -Iinclude  -pipe -Wall -pedantic -g -Wextra
CXXFLAGS := -std=gnu++11 -Iinclude -I/usr/local/freetype/2_12_1/include/freetype2
LDFLAGS  := -L/usr/local/lib  
LDLIBS   := -lm -ldl -lglfw -lfreetype

SRC_DIR:=src
BIN_DIR:=bin
OBJ_DIR:=bin/obj

EXE:=$(BIN_DIR)/TextRendering
SRC:=$(wildcard $(SRC_DIR)/*.c)
SRCCXX:=$(wildcard $(SRC_DIR)/*.cpp)
OBJ:=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
OBJ+=$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCCXX))


.PHONY: all

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

.PHONY: clean

clean:
	rm -r $(OBJ_DIR)


.PHONY: start

start:
	./$(EXE)
