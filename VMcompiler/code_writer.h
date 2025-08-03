#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "parser.h"

void vm_translator(comm* head, char* dir);
void translate_line(comm* head, FILE* file);

void write_arithmetic(comm* head, FILE* file);
void write_jump(comm* head, FILE* file);

void write_push(comm* head, FILE* file);
void get_constant(char* value, FILE* file);

void write_pop(comm* head, FILE* file);
void write_label(comm* head, FILE* file);
void write_goto(comm* head, FILE* file);
void write_if(comm* head, FILE* file);
void write_function(comm* head, FILE* file);
void write_return(comm* head, FILE* file);
void write_call(comm* head, FILE* file);

char* get_command_asm(char* line);


#endif