
SRC_DIR = ./src
LIBS_SRC_DIR = $(SRC_DIR)/libs

OBJECT_FILES_DIR = ./objects

BIN_DIR = ./bin
LIBS_DIR = $(BIN_DIR)/libs

LIBS_INSTALL_DIR = /usr/lib
BIN_INSTALL_DIR = /usr/bin

C_FLAGS = -I ./headers -I ./templates
LINK_FLAGS = -rdynamic -Wl,-rpath=$(LIBS_DIR) -ldl
C_SO_FLAGS = -I ./headers -I ./templates -shared -fPIC

all: libraries main
libraries: $(addprefix $(LIBS_DIR)/, $(patsubst %.c, %.so, $(notdir $(wildcard $(LIBS_SRC_DIR)/*.c))))
main: $(BIN_DIR)/main

$(BIN_DIR)/main: $(addprefix $(OBJECT_FILES_DIR)/, $(patsubst %.c, %.o, $(notdir $(wildcard $(SRC_DIR)/*.c)))) | $(BIN_DIR)
	gcc $(LINK_FLAGS) -o $@ $^

$(LIBS_DIR)/%.so: $(LIBS_SRC_DIR)/%.c | $(LIBS_DIR)
	gcc $(C_SO_FLAGS) -o $@ $^

$(OBJECT_FILES_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJECT_FILES_DIR)
	gcc -c $(C_FLAGS) -o $@ $^

clean:
	rm $(LIBS_DIR)/*
	rm -d $(BIN_DIR)/*
	rm $(OBJECT_FILES_DIR)/*

$(BIN_DIR):
	mkdir -p $@

$(LIBS_DIR):
	mkdir -p $@

$(OBJECT_FILES_DIR):
	mkdir -p $@
