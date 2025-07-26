#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>


void init(int hash_size);
void exit_program();
void set_file_exts();

FILE* write_file();
FILE* read_file();

void new_node_code(char* data, char type);
void transverse_code();
void destroy_node_code();

void first_pass(int hash_size);
char* next_line();
void register_symbol(char* line, int num, int hash_size);

int second_pass(FILE* file, char* line[]);

void a_instruction(char* line);
void c_instruction(char* line);

int c_register(char* reg);
int c_operand(char* operand);
int c_jump(char* jump);

int string_to_hash(char* string, int size);
int convert_to_bin(int dec);

#endif