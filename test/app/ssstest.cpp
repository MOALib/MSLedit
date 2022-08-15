#include "../../Src/msledit.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

std::string corrupt(std::string input, size_t corrupts){
    if(corrupts > input.size()){
        return input;
    }

    for(size_t counts = 0; counts < corrupts; counts++){
        size_t cbytw = std::rand() % input.size();
        char byt = 'a' + rand()%26;
        input[cbytw] = byt;
    }

    return input;
}

int main(int argc, char* argv[]){
    ((void)argc);
    ((void)argv);

    static std::string test;
    test = "Sarah's Marie Claire Black Office High Heels is broken and one of it's sides is torn up due to abusing it and battling it against other heels for a video, but it survived and she still wears it to this day due to destroying the other heel and so it remains the only heel she had as she put the rest in the muddy basement.";
    // test = "ed ";

    std::srand(time(nullptr));

    MXPSQL::MSLedit::MSLedit_Str normal(test);
    MXPSQL::MSLedit::MSLedit_Str corrupted(corrupt(test, 2));

    size_t normal_chksum = 0;
    size_t corrupt_chksum = 0;

    std::cout << "NORMAL> " << normal << "> " << (normal.stupidSimpleSummingCompare(corrupted, &normal_chksum, &corrupt_chksum) ? "true" : "false") << " " << normal_chksum << std::endl;
    std::cout << "CORRUPT> " << corrupted << "> " << (corrupted.stupidSimpleSummingCompare(normal, nullptr, nullptr) ? "true" : "false") << " " << corrupt_chksum << std::endl;

    return EXIT_SUCCESS;
}