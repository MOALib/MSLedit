.DEFAULT_GOALS := all
SRC_DIR=./bin-src
BIND_DIR=./bindings

.PHONY: all build bindings docs init clean

all: build

build:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) build CXX=$(CXX)

bindings:
	@echo "Delegating to makefile in $(BIND_DIR)"
	@cd $(BIND_DIR) && $(MAKE) bind

docs:
	@echo "Generating docs"
	@doxygen Doxyfile

init:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) init

	@echo "Delegating to makefile in $(BIND_DIR)"
	@cd $(BIND_DIR) && $(MAKE) init

clean:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) clean

	@echo "Delegating to makefile in $(BIND_DIR)"
	@cd $(BIND_DIR) && $(MAKE) clean

