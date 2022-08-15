#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

char* file_template = (char*) "/tmp/Sarah.XXXXXX";

int main(int argc, char** argv){
    ((void)argc);
    
    ((void)argv);

    /* Very scary very scary, it is illegal, extremly illegal, so illegal this should be an error. My compiler failed and it shuld failed, so it passed the test.
    char* illegal_mktemp(char* temp){
        return mktemp(temp);
    } */

    char* tempfile = mktemp(file_template);
    if(tempfile == NULL){
        printf("Nope, not a chance!");
    }
    else if(strcmp(tempfile, "")){
        printf("Literally empty ok");
    }
    else{
        printf("%s", tempfile);
    }
    return 0;
}