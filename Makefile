
BIN_DIR = ./bin
SRC_DIR = ./src
TEST_DIR = ./test

C_FLAGS = -I ./headers

%.test: $(TEST_DIR)/%.c $(SRC_DIR)/iotools.c $(SRC_DIR)/lexer.c
	@ gcc $(C_FLAGS) -o $(BIN_DIR)/% $^
	@ $(BIN_DIR)/%
