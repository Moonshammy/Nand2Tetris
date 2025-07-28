#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    char* data; //Should only store either A or C-instruction code (D=D+M or @15)
    int (*fp)(char*, int); //Points the function for whatever instruction is stored here
    struct node* next;  //Points to next code
}node_code;

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
void seperate_code(char* line, int hash_size);
char* next_line(FILE* file);
char* clean_line(char* line);

void new_node_code(char* data, char type);
node_code* create_node_code(char* data, char type);
void add_node_code(node_code* code);
void traverse_code();
void destroy_node_code();

static void init_predefined_symbols(int hashsize);
static void label_symbols(char* label, int hash_size, int address);

void add_symbol(symbol* sym, int hash_size);
symbol* create_symbol(char* line, int num);
symbol* create_var_symbol(char* line);
int get_symbol_address(char* line, int hash_size);
symbol* find_symbol(char* line, int hash_size);
void traverse_symbols(int hash_size);
void destroy_symbol(int hash_size);

void second_pass(int hash_size);

int a_instruction(char* line, int hash_size);
int c_instruction(char* line, int hash_size);

int c_parse(char* line, char** args);
int c_register(char* line);
int c_operand(char* line);
int c_jump(char* line);

int print_to_bin(FILE* file, int dec);
int string_to_hash(char* string, int size);


#endif