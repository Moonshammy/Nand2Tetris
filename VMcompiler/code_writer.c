#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_writer.h"
#include "parser.h"

typedef struct{
    char* command;
    char* assem;
}command;

const command commands_map[] = { {"add", "D+M"}, {"sub", "M-D"}, {"and", "D&M"}, {"or", "D|M"}, {"not", "!D"}, {"neg", "-D"}};

void vm_translator(comm* head, char* dir){
    FILE* w_file = fopen(dir, "w");
    while (head != NULL){
        translate_line(head, w_file);
        head = head->next;
    }
}

void translate_line(comm* head, FILE* file){
    switch (head->c_type){
        case (C_ARITHMETIC):
            write_arithmetic(head, file);
            break;
        case (C_JUMP):
            printf("Jump\n");
            break;
        case (C_PUSH):
            write_push(head, file);
            break;
        case (C_POP):
            printf("Pop\n");
            break;
        case (C_LABEL):
            printf("Label\n");
            break;
        case (C_GOTO):
            printf("Goto\n");
            break;
        case (C_IF):
            printf("If-GOTO\n");
            break;
        case (C_FUNCTION):
            printf("Function\n");
            break;
        case (C_RETURN):
            printf("Return\n");
            break;
        case (C_CALL):
            printf("Call\n");
            break;
    }
}

void write_arithmetic(comm* head, FILE* file){
    fprintf(file, "@SP\n");
    if (strcmp(head->command, "not") || strcmp(head->command, "net")){
        fprintf(file, "AM=M-1\n");
        fprintf(file, "D=M\n");
        fprintf(file, "A=A-1\n");
    }
    else{
        fprintf(file, "A=M-1\n");
    }
    fprintf(file, "DM=");
    fprintf(file, get_command_asm(head->command));
    fprintf(file, "\n");
}

void write_jump(comm* head, FILE* file);
void write_push(comm* head, FILE* file){
    if(strcmp(head->arg1, "constant") == 0){
        get_constant(head->arg2, file);
        fprintf(file, "@SP\n");
        fprintf(file, "AM=M+1\n");
        fprintf(file, "A=A-1\n");
        fprintf(file, "M=D\n");
    }
}

void get_constant(char* value, FILE* file){
    fprintf(file, "@");
    fprintf(file, value);
    fprintf(file, "\n");
    fprintf(file, "D=A\n");
}

void write_pop(comm* head, FILE* file);
void write_label(comm* head, FILE* file);
void write_goto(comm* head, FILE* file);
void write_if(comm* head, FILE* file);
void write_function(comm* head, FILE* file);
void write_return(comm* head, FILE* file);
void write_call(comm* head, FILE* file);

char* get_command_asm(char* line){
    for (int i = 0; i < 6; i++){
        if(strcmp(line, commands_map[i].command) == 0){
            return commands_map[i].assem;
        }
    }
}