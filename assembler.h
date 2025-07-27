#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>

typedef struct sym{
    char* name;
    int address;
    struct sym* next;
} symbol;

void init(int hash_size);
void exit_program(int hash_size);
void set_file_exts();

FILE* write_file();
FILE* read_file();

void first_pass(int hash_size);
char* next_line();

void new_node_code(char* data, char type);
void transverse_code();
void destroy_node_code();

static void init_predefined_symbols(int hashsize);
static void label_symbols(char* label, int hash_size, int address);

void add_symbol(symbol* sym, int hash_size);
symbol* create_symbol(char* line, int num);
symbol* create_var_symbol(char* line);
int get_symbol_address(char* line, int hash_size);
symbol* find_symbol(char* line, int hash_size);
void destroy_symbol(int hash_size);

void second_pass(int hash_size);

int a_instruction(char* line, int hash_size);
int c_instruction(char* line, int hash_size);

int c_register(char* line);
int c_operand(char* line);
int c_jump(char* line);

int print_to_bin(int dec);
int string_to_hash(char* string, int size);


#endif