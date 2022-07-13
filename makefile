.DEFAULT_GOALS := all
SRC_DIR=./bin-src
BIND_DIR=./bindings

.PHONY: all build bindings init clean

all: build

build:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) build

bindings:
	@echo "Delegating to makefile in $(BIND_DIR)"
	@cd $(./BIND_DIR) && $(MAKE) bindings

init:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) init

clean:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) clean

