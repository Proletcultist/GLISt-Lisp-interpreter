
BIN_DIR = ./bin
SRC_DIR = ./src
TEST_DIR = ./test

LIBS_DIR = /usr/lib

C_FLAGS = -g -I ./headers -I ./templates -rdynamic -Wl,-rpath=$(LIBS_DIR) -ldl

C_SO_FLAGS = -I ./headers -I ./templates -shared -fPIC

repl: $(BIN_DIR)/repl
std: $(LIBS_DIR)/GLIStSTD.so

$(BIN_DIR)/repl: $(TEST_DIR)/repltest.c $(filter-out $(SRC_DIR)/std.c, $(wildcard $(SRC_DIR)/*))
	gcc $(C_FLAGS) -o $@ $^


$(LIBS_DIR)/GLIStSTD.so: $(SRC_DIR)/std.c
	gcc $(C_SO_FLAGS) -o $@ $^
