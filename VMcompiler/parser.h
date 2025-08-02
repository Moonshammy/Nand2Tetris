#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

typedef enum {
    C_ARITHMETIC,
    C_JUMP,
    C_POP,
    C_PUSH,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL
}C_Type;

typedef struct COMM {
    C_Type c_type;
    char* command;
    char* arg1;
    char* arg2;
    struct COMM* next;
} comm;

comm* parse_files(char* dir);

char** get_files(char* folder_path);
char* string_copy(char* src);
FILE* open_file(char* file_path);

void parse_file(FILE* file);
char* next_line(FILE* file);
char* strip_spacing(char* line);
char** parse_string(char* line);
C_Type get_command_type(char* line);

void add_to_queue(char** s_line, C_Type type);
comm create_node(char** s_line, C_Type);
void add_node(comm node);

#endif