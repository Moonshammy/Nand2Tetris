#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctype.h>

#include "parser.h"

typedef struct{
    const char* command;
    C_Type type;
} types;

const types types_map[] = {
    {"add", C_ARITHMETIC},
    {"sub", C_ARITHMETIC},
    {"neg", C_ARITHMETIC},
    {"and", C_ARITHMETIC},
    {"or", C_ARITHMETIC},
    {"not", C_ARITHMETIC},
    {"gt", C_JUMP},
    {"lt", C_JUMP},
    {"eq", C_JUMP},
    {"push", C_PUSH},
    {"pop", C_POP},
    {"label", C_LABEL},
    {"goto", C_GOTO},
    {"if-goto", C_IF},
    {"function", C_FUNCTION},
    {"call", C_CALL},
    {"return", C_RETURN},

};

comm* parse_vm(char* dir){
    static comm* head;
    char** files;
    
    files = get_files(dir);
    head = parse_files(head, files);
    
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

comm* parse_files(comm* head, char** files){
    int i = 0;
    FILE* file;
    while (files[i] != NULL){
        file = open_file(files[i]);
        head = parse_file(head, file);
        fclose(file);
        i++;
    }
    return head;
}

FILE* open_file(char* file_path){
    FILE* file = fopen(file_path, "r");
    if (file == NULL){
        perror("Error opening file");
        exit(1);
    }
    return file;
}

comm* parse_file(comm* head, FILE* file){
    char* line = next_line(file);
    char* s_line[3];
    C_Type type;
    while (line != NULL){
        line = strip_spacing(line);
        if (parse_string(line, s_line) != NULL){
            type = get_command_type(s_line[0]);
            head = add_to_queue(s_line, type, head);
        } 
        line = next_line(file);
    }
    free(line);
    return head;
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

C_Type get_command_type(char* line){
    for (int i = 0; i < 17; i++){
        if (strcmp(line, types_map[i].command) == 0){
            return types_map[i].type;
        }
    }
}

comm* add_to_queue(char** s_line, C_Type type, comm* head){
    comm* node = create_node(s_line, type);
    head = add_node(node, head);
    return head;
}
comm* create_node(char** s_line, C_Type type){
    comm* new_node = calloc(1, sizeof(comm));
    new_node->c_type = type;
    new_node->command = s_line[0];
    new_node->arg1 = s_line[1];
    new_node->arg2 = s_line[2];
    new_node->next = NULL;
}

comm* add_node(comm* node, comm* head){
    comm* curr = head;
    if (head == NULL){
        head = node;
    }
    else{
        while (curr->next != NULL){
            curr = curr->next;
        }
        curr->next = node;
    }
    return head;
}

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
    comm* head;
    char* dir = "..\\..\\project_files\\8\\StaticsTest";
    head = parse_vm(dir);
    comm* curr = head;
    while(curr != NULL){
        printf("ENUM: %d, command: %s, arg1: %s, arg2: %s\n", curr->c_type, curr->command, curr->arg1, curr->arg2);
        curr = curr->next;
    }
    printf("Program complete\n");

}