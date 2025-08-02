#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctype.h>

#include "parser.h"

 //For commands queue


comm* parse_vm(char* dir){
    comm* head;
    char** files;
    
    files = get_files(dir);
    parse_files(head, files);
    
    free(files);
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
                files[i] = directory_copy(folder_path, dirent->d_name);
                i++;
            }
        }
    }
    closedir(dir);
    files[i] = NULL;
    int j = 0;
    return files;
}

void parse_files(comm* head, char** files){
    int i = 0;
    FILE* file;
    while (files[i] != NULL){
        file = open_file(files[i]);
        parse_file(head, file);
        fclose(file);
        i++;
    }
}

FILE* open_file(char* file_path){
    FILE* file = fopen(file_path, "r");
    if (file == NULL){
        perror("Error opening file");
        exit(1);
    }
    return file;
}

void parse_file(comm* head, FILE* file){
    char* line = next_line(file);
    char* s_line[3];
    while (line != NULL){
        line = strip_spacing(line);
        if (parse_string(line, s_line) != NULL){
            printf("%s %s %s\n", s_line[0], s_line[1], s_line[2]);
        } 
        line = next_line(file);
    }
    free(line);
}

char* next_line(FILE* file){
    char buffer[1024];
    char* line = calloc(1024, sizeof(char));
    if (fgets(buffer, sizeof(buffer), file) != NULL){
        strcpy(line, buffer);
        return line;
    }
    free(line);
    return NULL;
}

char* strip_spacing(char* line){
    char* end = line + strlen(line) -1;
    if (*line == 0){
        return line;
    }
    while (isspace(line[0])){
        line++;
    }
    while(isspace(end[0])){
        end--;
    }
    end[1] = '\0';
    return line;
}

char* parse_string(char* line, char** s_line){
    if (line[0] == '/' || line[0] == '\0' || line[0] == '\n'){
        return NULL;
    }

    s_line[0] = calloc(10, sizeof(char)); //command
    s_line[1] = calloc(256, sizeof(char)); //arg1
    s_line[2] = calloc(10, sizeof(char)); //arg2
    
    int i = 0;
    while(line[i] != '\0' && line[i] != ' '){
        s_line[0][i] = line[i];
        i++;
    }
    line += i + 1;
    i = 0;
    while(line[i] != '\0' && line[i] != ' '){
        s_line[1][i] = line[i];
        i++;
    }
    line += i + 1;
    i = 0;
    while(line[i] != '\0' && line[i] != ' '){
        s_line[2][i] = line[i];
        i++;
    }
    return "Pass";
}

C_Type get_command_type(char* line);

void add_to_queue(char** s_line, C_Type type);
comm create_node(char** s_line, C_Type);
void add_node(comm node);

char* directory_copy(char* folder_path, char* src){
    int len = strlen(folder_path)+strlen(src)+2;
    char* string = calloc(len, sizeof(char));

    for (int i = 0; i < strlen(folder_path); i++){
        string[i] = folder_path[i];
    }
    string[strlen(folder_path)] = '\\';
    
    int i = 0;
    for (int j = strlen(folder_path)+1; j < len; j++){
        string[j] = src[i];
        i++;
    }
    return string;
}

void main(){
    comm* test;
    char* dir = "..\\..\\project_files\\8\\StaticsTest";
    parse_vm(dir);
    printf("Program complete\n");
}