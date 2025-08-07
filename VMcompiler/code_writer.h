#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "parser.h"

void vm_translator(comm* head, char* dir);
void init(FILE* file, char* dir);

void set_file(char* dir);
FILE* get_file();

void bootstrap_init(FILE* file);
void jump_init(FILE* file);
void end(FILE* file);

void translate_line(comm* head, FILE* file);

void push(FILE* file);
void pop(FILE* file);
void constant(char* value, FILE* file);

void write_arithmetic(char* command, FILE* file);
void write_jump(char* command, FILE* file);

void write_push(char* arg1, char* arg2, FILE* file);
void get_pointer_address(char* arg2, FILE* file);
void get_static_address(char* arg2, FILE* file);
void get_temp_value(char* value, FILE* file);
void get_address_value(char* value, FILE* file);
void store_address_index(char* value, FILE* file);

void write_pop(char* arg1, char* arg2, FILE* file);
void write_label(char* arg1, FILE* file);
void write_goto(char* arg1, FILE* file);
void write_if(char* arg1, FILE* file);
void write_function(char* arg1, char* arg2, FILE* file);
void zero_local(char* arg2, FILE* file);

void write_return(FILE* file);
void write_call(char* arg1, char* arg2, FILE* file);

char* get_command_asm(char* line);

#endif