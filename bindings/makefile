# Copied from https://github.com/MOALib/mshar/blob/master/c/binding/makefile and modified

MSLEDIT_SWIG=msledit.swigi
MSLEDIT_H=../src/msledit.hpp
thisSrc=src

CSBIND_DIR=csharp
PYBIND_DIR=py

.DEFAULT_GOAL := all

SWIGFLAGS=-c++

.PHONY: cls  bind   csharp python  clean init

cls:
	@clear || cls


bind: cls csharp python

csharp:
	@echo "Generating C# Bindings"
	@cp $(MSLEDIT_SWIG) $(CSBIND_DIR)
	@cd $(CSBIND_DIR) && mkdir $(CSBIND_DIR) 2> /dev/null || true
	@-cd $(CSBIND_DIR) && swig $(SWIGFLAGS) -csharp -namespace MXPSQL.MSLeditSharp -outdir $(CSBIND_DIR) $(MSLEDIT_SWIG)

python:
	@echo "Generating Python Bindings"
	@cp $(MSLEDIT_SWIG) $(PYBIND_DIR)
	@-cd $(PYBIND_DIR) && swig $(SWIGFLAGS) -python -outdir . $(MSLEDIT_SWIG)


clean:
	@echo "Cleaning C# binding"
	@-rm -rf $(CSBIND_DIR)/* 2> /dev/null || true

	@echo "Cleaning Python binding"
	@-rm -rf $(PYBIND_DIR)/* 2> /dev/null || true

	@echo "Removing copied source directory"
	@-rm -rf $(thisSrc) 2> /dev/null || true

init:
	@echo "Initializing"

	@echo "Making source directory and copying source file"
	@-mkdir $(thisSrc) 2> /dev/null || true
	@-cp $(MSLEDIT_H) $(thisSrc)

	@echo "Making C# bindings directory"
	@-mkdir $(CSBIND_DIR) 2> /dev/null || true

	@echo "Making Python bindings directory"
	@-mkdir $(PYBIND_DIR) 2> /dev/null || true