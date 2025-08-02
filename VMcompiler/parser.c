#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

static comm* head; //For commands queue

comm* parse_files(char* dir);

void get_files(char* dir);
FILE* open_file(char* dir);

void parse_file(FILE* file);
char* next_line(FILE* file);
char* strip_spacing(char* line);
char** parse_string(char* line);
C_Type get_command_type(char* line);

void add_to_queue(char** s_line, C_Type type);
comm create_node(char** s_line, C_Type);
void add_node(comm node);

void main(){
    printf("Test");
}