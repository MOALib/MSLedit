.DEFAULT_GOALS := all
SRC_DIR=./bin-src
PKG_ENV_DIR=./pkgenv
BIND_DIR=./bindings

THREADS:=8

.PHONY: all build bindings mkpkg docs init clean

all: build

build:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) -j$(THREADS) build CXX=$(CXX)

bindings:
	@echo "Delegating to makefile in $(BIND_DIR)"
	@cd $(BIND_DIR) && $(MAKE) -j$(THREADS) bind

mkpkg:
	@echo "Delegating to makefile in $(PKG_ENV_DIR)"
	@cd $(PKG_ENV_DIR) && $(MAKE) -j$(THREADS)

docs:
	@echo "Generating docs"
	@doxygen Doxyfile

init:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) -j$(THREADS) init

	@echo "Delegating to makefile in $(BIND_DIR)"
	@cd $(BIND_DIR) && $(MAKE) -j$(THREADS) init

clean:
	@echo "Delegating to makefile in $(SRC_DIR)"
	@cd $(SRC_DIR) && $(MAKE) -j$(THREADS) clean

	@echo "Delegating to makefile in $(BIND_DIR)"
	@cd $(BIND_DIR) && $(MAKE) -j$(THREADS) clean

