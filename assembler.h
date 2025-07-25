#include <stdio.h>
#include <stdlib.h>

void set_file_exts(char* filename);

FILE* write_file();
FILE* read_file();

int first_pass(FILE* file, char* code[]);
char* next_line(FILE* file);

int second_pass(FILE* file, char* line[]);

void a_instruction(FILE* file, char* line);

void c_instruction(FILE* file, char* line);
int c_register(char* reg);
int c_operand(char* operand);
int c_jump(char* jump);

int convert_to_bin(int dec);
