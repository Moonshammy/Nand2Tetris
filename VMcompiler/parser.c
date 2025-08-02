#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

#include "parser.h"

 //For commands queue
static comm* head;

comm* parse_files(char* dir){
    char** files;
    files = get_files(dir);
    return head;
}

char** get_files(char* folder_path){
    DIR* dir = opendir(folder_path);
    if (dir == NULL){
        perror("Error opening directory");
        exit(1);
    }
    
    char** files = calloc(256, sizeof(char**));
    char* line;
    struct dirent* dirent;
    int i = 0;

    while ((dirent = readdir(dir)) != NULL){
        if (dirent->d_namlen > 3){ 
            if (!strcmp(dirent->d_name + dirent->d_namlen - 3, ".vm")){
                files[i] = string_copy(dirent->d_name);
                i++;
            }
        }
    }

    closedir(dir);
    files[i] = NULL;
    int j = 0;
    return files;
}

char* string_copy(char* src){
    char* string = calloc(strlen(src), sizeof(char));
    for (int i = 0; i < strlen(src); i++){
        string[i] = src[i];
    }
    return string;
}

FILE* open_file(char* file_path);

void parse_file(FILE* file);
char* next_line(FILE* file);
char* strip_spacing(char* line);
char** parse_string(char* line);
C_Type get_command_type(char* line);

void add_to_queue(char** s_line, C_Type type);
comm create_node(char** s_line, C_Type);
void add_node(comm node);

void main(){
    comm* test;
    char* dir = "..\\..\\project_files\\8\\StaticsTest";
    parse_files(dir);
    printf("Program complete\n");
}