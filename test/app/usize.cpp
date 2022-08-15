#include <iostream>
#include <string>
#include <cstdlib>

#include "../../Src/msledit.hpp"

int main(int argc, char* argv[]){
    ((void)argc);
    ((void)argv);

    MXPSQL::MSLedit::MSLedit_Str editor;

    if(sizeof(editor) >= std::string::npos){
        std::cerr << "bad type size, undefined behaviour has occured. Terminating. (This has gone unstable)" << std::endl;
        std::abort(); // abortion
    }
    else{
        std::cout << "editor size is " << sizeof(editor) << " bytes" << std::endl;
    }

    return EXIT_SUCCESS;
}