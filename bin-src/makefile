CXX:=g++
MINGWCXX=i686-w64-mingw32-$(CXX)

# change -std=c++11 to -std=gnu++11 to enable POSIX functions
STD=c++11

CXXFLAGS=-pipe -std=$(STD) -pedantic -Wall -Wextra -Werror -fdiagnostics-color -Og -g
STATIC_FLAG=-static -lstdc++ -static-libstdc++
RL_CFLAGS=-lreadline -lncurses
# -fsanitize=pointer-compare,pointer-subtract,address,leak,undefined

BIN_SRC=msledit.cpp
GPL_BIN_SRC=msledit-rg3.cpp
LIB_SRC=msleditlib.cpp

BIN_DIR=../out
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
	$(CXX) $(STATIC_FLAG) $(CXXFLAGS) $(ECFLAGS) $(BIN_SRC) -o $(BIN_BIN) # make binary

	@# Non essential, this is an example for hooking up readline
	-$(CXX) $(STATIC_FLAG) $(CXXFLAGS) $(ECFLAGS) $(GPL_BIN_SRC) -o $(GPL_BIN_BIN) $(RL_CFLAGS) # make GPLd binary

	@printf "\n\n"

makenbin:
	@# Non essential due to being native versions ok
	-$(MINGWCXX) $(STATIC_FLAG) $(CXXFLAGS) $(ECFLAGS) $(BIN_SRC) -o $(BIN_MINGW-BIN) # make native binary
	
	@# separate stuff ok
	-$(MINGWCXX) $(STATIC_FLAG) $(CXXFLAGS) $(ECFLAGS) $(GPL_BIN_SRC) -o $(GPL_MINGW_BIN) $(RL_CFLAGS) # make native GPLd binary

	@printf "\n\n"

makelib:
	: # make library


init:
	@echo "Initializing build environment"
	@-mkdir $(BIN_DIR)

clean:
	@echo "Cleaning up build environment"
	@-rm -rf $(BIN_DIR)