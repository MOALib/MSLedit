.DEFAULT_GOALS := all
SRC_DIR=./bin-src

.PHONY: all build init clean

all: build

build:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) build

init:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) init

clean:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) clean

