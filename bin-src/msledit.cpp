/**
 * @file msledit.cpp
 * @author MXPSQL
 * @brief Binary source code of MSLedit
 * @version 0.1
 * @date 2022-06-29
 * 
 * @copyright
 * 
 * Copyright (c) 2022 MXPSQL
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <new>
#include <string>
#include <vector>
#include "../Src/msledit.hpp"

/**
 * @brief Main method of the MSLedit binary (test environment)
 * 
 * @param argc arguments count
 * @param argv arguments v?
 * @return int status code
 */
int main(int argc, char** argv){
    int status = EXIT_SUCCESS;
    char* emergency_memory = new char[sizeof(char)*(16*1024*1024)];
    MXPSQL::MSLedit::MSLedit editor{"big", "pee", "pee", "big pee pee", "a big is a bee as a pig", "a pee is a pig as a pee", "pee", "pee gets attacked", "pissed pee", "a pissed pee is an angry pee", "haha peach amibo gets smashed, arm broke of, depatitated, fallen of base", "nokia, a nokia 3310 i think", "western electric 500 telephone", "walt has a big head"};
    std::string file = "";
    std::string prompt = "";
    bool nbanner = false;
    bool cleardoc = false;

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
            << "-c, --clear-doc\tClear test document" << std::endl
            << std::endl;
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
        else if(arg == "-c" || arg == "--clear-doc"){
            cleardoc = true;
        }
        else{
            std::cerr << "What is this stupid bingus of an argument called " << arg << "? " << std::endl;
            status = EXIT_FAILURE;
            return status;
        }
    }


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

                delete[] emergency_memory;
                emergency_memory = nullptr;

                return status;
            }
        }
        if(!prompt.empty()){
            editor.setKey("prompt", prompt);
        }
        editor.setKey(editor.nobanner, ((nbanner) ? "true" : "false"));
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
            return 0;
        }; */
        status = editor.repl();
    }
    catch(std::runtime_error& re){
        std::cerr << re.what() << std::endl;
        status = EXIT_FAILURE;
    }

    delete[] emergency_memory;
    emergency_memory = nullptr;

    return status;
}
