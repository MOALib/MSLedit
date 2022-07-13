CXX=g++
MINGWCXX=i686-w64-mingw32-$(CXX)
CXXFLAGS=-static -std=c++11 -pedantic -Wall -Wextra -Werror -fdiagnostics-color -Og -g

BIN_SRC=msledit.cpp
LIB_SRC=msleditlib.cpp

BIN_DIR=bin
BIN_BIN=$(BIN_DIR)/msledit.exe
BIN_MINGW-BIN=$(BIN_DIR)/msledit-n.exe
LIB_BIN=$(BIN_DIR)/libmsledit.o

.PHONY: cls  build makebin makenbin makelib  init clean

cls:
	@clear || cls


build: cls makebin makenbin makelib

makebin:
	$(CXX) $(CXXFLAGS) $(ECFLAGS) $(BIN_SRC) -o $(BIN_BIN)

makenbin:
	$(MINGWCXX) $(CXXFLAGS) $(ECFLAGS) $(BIN_SRC) -o $(BIN_MINGW-BIN)

makelib:
	:


init:
	@echo "Initializing build environment"
	@-mkdir $(BIN_DIR)

clean:
	@echo "Cleaning up build environment"
	@-rm -rf $(BIN_DIR)