#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "parser.h"

void vm_translator(comm* head, char* dir);
void init(char* dir);

void set_file(char* dir);
FILE* get_file();

void bootstrap_init();
void jump_init();
void end();

void translate_line(comm* head);

void push();
void pop();
void constant(char* value);

void write_arithmetic(char* command);
void write_jump(char* command);

void write_push(char* arg1, char* arg2);
void write_pop(char* arg1, char* arg2);

void get_pointer_address(char* arg2);
void get_static_address(char* arg2);
void get_temp_value(char* value);
void get_address_value(char* value);
void store_address_index(char* value);

void write_label(char* arg1);
void write_goto(char* arg1);
void write_if(char* arg1);
void write_function(char* arg1, char* arg2);
void zero_local(char* arg2);

void write_return();
void write_call(char* arg1, char* arg2);

char* get_command_asm(char* line);

#endif