CXX:=g++
MINGWCXX=i686-w64-mingw32-$(CXX)
CXXFLAGS=-L/usr/lib    -pipe -static -std=c++11 -pedantic -Wall -Wextra -Werror -fdiagnostics-color -Og -g

RL_CFLAGS=-lreadline
# -fsanitize=pointer-compare,pointer-subtract,address,leak,undefined

BIN_SRC=msledit.cpp
GPL_BIN_SRC=msledit-rg3.cpp
LIB_SRC=msleditlib.cpp

BIN_DIR=bin
BIN_BIN=$(BIN_DIR)/msledit.exe
GPL_BIN_BIN=$(BIN_DIR)/msledit-rg3.exe
BIN_MINGW-BIN=$(BIN_DIR)/msledit-n.exe
GPL_MINGW_BIN=$(BIN_DIR)/msledit-rg3n.exe
LIB_BIN=$(BIN_DIR)/libmsledit.o

.PHONY: cls  build makebin makenbin makelib  init clean

cls:
	@clear || cls


build: cls makebin makenbin makelib

makebin:
	$(CXX) $(CXXFLAGS) $(ECFLAGS) $(BIN_SRC) -o $(BIN_BIN) # make binary

	@# Non essential, this is an example for hooking up readline
	-$(CXX) $(CXXFLAGS) $(RL_CFLAGS) $(ECFLAGS) $(GPL_BIN_SRC) -o $(GPL_BIN_BIN) # make GPLd binary
	@printf '%s' '\n\n'

makenbin:
	@# Non essential due to being native versions ok
	- $(MINGWCXX) $(CXXFLAGS) $(ECFLAGS) $(BIN_SRC) -o $(BIN_MINGW-BIN) # make native binary
	- $(MINGWCXX) $(CXXFLAGS) $(RL_CFLAGS) $(ECFLAGS) $(GPL_BIN_SRC) -o $(GPL_BIN_BIN) # make native GPLd binary
	@printf '%s' '\n\n'

makelib:
	: # make library


init:
	@echo "Initializing build environment"
	@-mkdir $(BIN_DIR)

clean:
	@echo "Cleaning up build environment"
	@-rm -rf $(BIN_DIR)