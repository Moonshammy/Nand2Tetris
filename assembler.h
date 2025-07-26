#include <stdio.h>
#include <stdlib.h>


void init();
void exit_program();
void set_file_exts();

FILE* write_file();
FILE* read_file();

void new_node_code(char* data, char type);
void transverse_code();
void destroy_node_code();

void first_pass();
void parse_file();
char* next_line();

int second_pass(FILE* file, char* line[]);

void a_instruction(char* line);
void c_instruction(char* line);

int c_register(char* reg);
int c_operand(char* operand);
int c_jump(char* jump);

int convert_to_bin(int dec);
