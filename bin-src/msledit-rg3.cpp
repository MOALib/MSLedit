/**
 * @file msledit.cpp
 * @author MXPSQL
 * @brief GPLd version of binary source code of MSLedit. GPLd due to readline.
 * @version 0.1
 * @date 2022-06-29
 * 
 * @copyright
 * 
 * MSLedit-G3, a GPL version of MSLedit.
 * Copyright (C) 2022  MXPSQL
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <new>
#include <string>
#include <memory>
#include <vector>
#include "../Src/msledit.hpp"

#include <readline/readline.h>
#include <readline/history.h>

auto rl_handler = [](std::istream& input, std::string& out) -> bool {
    char* rl_str = (char*) NULL;
    rl_str = readline("");
    (void(input));
    bool status = true;
    if(!(rl_str == NULL || rl_str == nullptr) && (rl_str && *rl_str)){
        out = std::string(rl_str);
        add_history(rl_str);
    }
    else{
        status = false;
    }
    free(rl_str);
    return status;
};

auto gplInfo = []() -> std::string {
    return "\nThis version is licensed under GPL due to using readline. Your copy of the license is in the sources.";
};

/**
 * @brief Main method of the MSLedit binary, GPLd
 * 
 * @param argc arguments count
 * @param argv arguments v?
 * @return int status code
 */
int main(int argc, char** argv){
    int status = EXIT_SUCCESS;
    std::unique_ptr<char[]> emergency_memory(new char[sizeof(char)*(16*1024*1024)]);
    MXPSQL::MSLedit::MSLedit_Str editor{"big", "pee", "pee", "big pee pee", "a big is a bee as a pig", "a pee is a pig as a pee", "pee", "pee gets attacked", "pissed pee", "a pissed pee is an angry pee", "haha peach amibo gets smashed, arm broke of, depatitated, fallen of base", "nokia, a nokia 3310 i think", "western electric 500 telephone", "walt has a big head"};
    std::string file = "";
    std::string prompt = "";
    bool nbanner = false;
    bool cleardoc = false;
    bool ncolor = false;

    if(sizeof(editor) >= std::string::npos){
        std::cerr << "bad type size, undefined behaviour has occured. Terminating. (This has gone unstable)" << std::endl;
        emergency_memory.reset();
        std::abort(); // abortion
    }

    for(int i = 1; i < argc; i++){
        std::string arg = std::string(argv[i]);
        bool nextOptAvailable = ((i+1)<argc);
        bool nextOptNextAvailable((i+2)<argc);
        if(arg == "-h" || arg == "--help" || arg == "-?"){
            std::cout << "MSLedit CLI Editor" << std::endl
            << "Flags Help" << std::endl
            << "Usage example: " << argv[0] << " -?" << std::endl
            << "-h, --help, -? \t Show this fockin help screen" << std::endl
            << "-f, --file\tOpen a file" << std::endl
            << "-p, --prompt\tSet the prompt" << std::endl
            << "-nb, --no-banner\tDisable welcome banner" << std::endl
            << "-nc, --no-color\tDisable color on prompt, also if term is not supported" << std::endl
            << "-c, --clear-doc\tClear test document" << std::endl
            << std::endl;
            emergency_memory.reset();
            return status;
        }
        else if(arg == "-f" || arg == "--file"){
            if(nextOptAvailable){
                file = argv[i+1];
                if(nextOptNextAvailable){
                    i++;
                }
                else{
                    break;
                }
            }
            else{
                std::cerr << "Missing arguments to " << arg << std::endl;
                status = EXIT_FAILURE;
                emergency_memory.reset();
                return status;
            }
        }
        else if(arg == "-p" || arg == "--prompt"){
            if(nextOptAvailable){
                prompt = argv[i+1];
                if(nextOptNextAvailable){
                    i++;
                }
                else{
                    break;
                }
            }
            else{
                std::cerr << "Missing arguments to " << arg << std::endl;
                status = EXIT_FAILURE;
                return status;
            }
        }
        else if(arg == "-nb" || arg == "--no-banner"){
            nbanner = true;
        }
        else if(arg == "-nc" || arg == "--no-color"){
            ncolor = true;
        }
        else if(arg == "-c" || arg == "--clear-doc"){
            cleardoc = true;
        }
        else{
            std::cerr << "What is this stupid bingus of an argument called " << arg << "? " << std::endl;
            status = EXIT_FAILURE;
            emergency_memory.reset();
            return status;
        }
    }

    // std::cout << editor[30] << std::endl;


    try{
        editor.appendAtLine(5, "eee\nd");
        editor.appendAtLine(5, "eeee, reee\ndddd, o\nmarie claire");
        // editor.insertAtLine(5, "eeee, reee\ndddd, o\nmarie claire");
        editor.editLine(1, "aeaeadeeddee\neeccecle\nbig");
        /* {
            for(std::string s : editor.split("walt")){
                std::cout << s << std::endl;
            }
        } */
        editor.append("dietz nuts. eeeeeee");
        // editor.insert(3, "\nnokia\nprincess peach amibo get smashed\n");
        editor.appendNewLine();
        editor.append(nullptr);
        // editor.insert(3, NULL);
        editor.appendNewLine();
        editor.append_printf("%s and %p at %i during %a and characters printed are (REDACTED) while %p eats the character %c and string %s\n", "tes yes", (void*)1, 21, 1.1, (void*)'\v', 'd', "str");
        editor.append(std::string::npos);
        if(cleardoc){
            editor.clear();
        }
        if(!file.empty()){
            try{
                editor.readFile(file);
            }
            catch(std::runtime_error& re2){
                std::cerr << "what is this, you open something nonexistent or broken called '" << file << "'?" << std::endl;
                status = EXIT_FAILURE;

                emergency_memory.reset();

                return status;
            }
        }
        if(!prompt.empty()){
            editor.setKey("prompt", prompt);
        }
        editor.setKey(editor.nobanner, ((nbanner) ? "true" : "false"));
        editor.setKey(editor.nocolor, ((ncolor) ? "true" : "false"));
        /* {
            editor.print(true, std::cout, -1, -1);
            for(size_t i = 1; i < editor.length(); i++){
                auto s = editor.getGridIndexFromStringIndex(i);
                std::cout << s.first << "|" << s.second << "> '" << editor.charAtPosition(i) << "' == '" << editor.charAtGrid(s.first, s.second) << "'" << std::endl;

                if(editor.charAtPosition(i) != editor.charAtGrid(s.first, s.second)){
                    std::cerr << "not the same" << std::endl;
                    status = EXIT_FAILURE;
                    return status;
                }
            }
        } */
        /* editor.replBeginHandler = [](std::string begin, std::vector<std::string> args, size_t arglen, std::ostream& out, std::istream& in, std::ostream& err) -> int {
            out << "no command line commands." << std::endl
            << "Begin: " << begin << std::endl
            << "Extra args" << std::endl;
            for(size_t i = 0; i < arglen; i++){
                out << "(" << std::to_string(i) << ")> " << args[i] << std::endl;
            }
            err << "you do not do that sem mistak ok ok ok" << std::endl;
            in.get();
            return EXIT_SUCCESS;
        }; */
        editor.replHelpHandler = gplInfo;
        status = editor.repl((editor.keyExists(editor.nprompt) ? editor.getKey(editor.nprompt) : "> "), std::cout, std::cin, rl_handler, std::cerr);
    }
    catch(std::runtime_error& re){
        std::cerr << re.what() << std::endl;
        status = EXIT_FAILURE;
    }

    return status;
}
