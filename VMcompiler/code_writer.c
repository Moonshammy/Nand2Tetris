#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code_writer.h"
#include "parser.h"

typedef struct{
    void* key;
    char* value;
}map;

const map commands_map[6] = { {"add", "D+M"}, {"sub", "M-D"}, {"and", "D&M"}, {"or", "D|M"}, {"not", "!D"}, {"neg", "-D"}};
const map address_name_map[4] = {{"this", "@THIS"}, {"that", "@THAT"}, {"local", "@LCL"}, {"argument", "@ARG"}};
const map temp_map[8] = {{"0", "@R5"}, {"1", "@R6"}, {"2", "@R7"}, {"3", "@R8"}, {"4", "@R9"}, {"5", "@R10"}, {"6", "@R11"}, {"7", "@R12"}};
int static_map[240];



void vm_translator(comm* head, char* dir){ 
    
    FILE* w_file = fopen(dir, "w");
    //init(w_file);
    while (head != NULL){
        translate_line(head, w_file);
        head = head->next;
    }
}

void init(FILE* file){
    bootstrap_init(file);
    jump_init(file);
    sys_init(file);
}

void bootstrap_init(FILE* file){
    get_constant("256", file);
    fprintf(file, "@SP\n");
    fprintf(file, "M=D");
}

void sys_init(FILE* file){}

void jump_init(FILE* file){
    char* start[] = {"start_gt", "start_lt", "start_eq"};
    char* true[] = {"true_gt", "true_lt", "true_eq"};
    char* end[] = {"end_gt", "end_lt", "end_eq"};
    char* type[] = {"JGT", "JLT", "JEQ"};
    for (int i = 0; i < 3; i++){
        fprintf(file, "\n//start of %s\n", type[i]);
        fprintf(file, "(%s)\n", start[i]);
        fprintf(file, "@%s\n", true[i]);
        fprintf(file, "D;%s\n", type[i]);
        fprintf(file, "DM=0\n");
        fprintf(file, "@%s\n", end[i]);
        fprintf(file, "0;JMP\n");
        fprintf(file, "(%s)\n", true[i]);
        fprintf(file, "DM=-1\n");
        fprintf(file, "(%s)\n", end[i]);
        fprintf(file, "@R13\n");
        fprintf(file, "A=M\n");
        fprintf(file, "0;JMP\n");
    }
}

void translate_line(comm* head, FILE* file){
    switch (head->c_type){
        case (C_ARITHMETIC):
            write_arithmetic(head, file);
            break;
        case (C_JUMP):
            write_jump(head, file);
            break;
        case (C_PUSH):
            write_push(head, file);
            break;
        case (C_POP):
            write_pop(head, file);
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

void push(FILE* file){
    fprintf(file, "@SP\n");
    fprintf(file, "AM=M+1\n");
    fprintf(file, "A=A-1\n");
    fprintf(file, "M=D\n");
}
void pop(FILE* file){
    fprintf(file, "@SP\n");
    fprintf(file, "AM=M-1\n");
    fprintf(file, "D=M\n");
}

void get_constant(char* value, FILE* file){
    fprintf(file, "@");
    fprintf(file, value);
    fprintf(file, "\n");
    fprintf(file, "D=A\n");
}

void write_arithmetic(comm* head, FILE* file){
    fprintf(file, "\n//%s\n", head->command);  //DEBUG
    fprintf(file, "@SP\n");
    if (strcmp(head->command, "not") != 0 && strcmp(head->command, "neg") != 0){
        fprintf(file, "AM=M-1\n");
        fprintf(file, "D=M\n");
        fprintf(file, "A=A-1\n");
    }
    else{
        fprintf(file, "A=M-1\n");
    }
    fprintf(file, "MD=");
    fprintf(file, get_command_asm(head->command));
    fprintf(file, "\n");
}

void write_jump(comm* head, FILE* file){
    char* jump;
    static int counter = 0;
    char* jump_map[] = {"gt", "JGT", "lt", "JLT", "eq", "JEQ"};
    
    
    for (int i = 0; i < 6; i += 2){
        if (strcmp(jump_map[i], head->command) == 0){
            jump = jump_map[i+1];
        }
    }
    fprintf(file, "\n//Start jump %s\n", jump);
    fprintf(file, "@SP\n");
    fprintf(file, "AM=M-1\n");
    fprintf(file, "D=M\n");
    fprintf(file, "A=A-1\n");
    fprintf(file, "D=M-D\n");
    fprintf(file, "@true%d\n", counter);
    fprintf(file, "D;%s\n", jump);
    fprintf(file, "D=0\n");
    fprintf(file, "@end_jump%d\n", counter);
    fprintf(file, "0;JMP\n");
    fprintf(file, "(true%d)\n", counter);
    fprintf(file, "D=-1\n");
    fprintf(file, "(end_jump%d)\n", counter);
    fprintf(file, "@SP\n");
    fprintf(file, "A=M-1\n");
    fprintf(file, "M=D\n");
    counter++;
}

//Temp hold
/*void write_jump(comm* head, FILE* file){
    static int rtn = 0;
    char* jump;
    char* jump_map[] = {"gt", "JGT", "lt", "JLT", "eq", "JEQ"};
    for (int i = 0; i < 6; i += 2){
        if (strcmp(jump_map[i], head->command) == 0){
            jump = jump_map[i+1];
        }
    }
    fprintf(file, "\n//Start of jump %s\n", jump);
    fprintf(file, "@returnjump%d\n", rtn);
    fprintf(file, "D=A\n");
    fprintf(file, "@R13\n");
    fprintf(file, "M=D\n");
    fprintf(file, "@SP\n");
    fprintf(file, "AM=M-1\n");
    fprintf(file, "D=M\n");
    fprintf(file, "A=A-1\n");
    fprintf(file, "MD=M-D\n");
    fprintf(file, "@start_%s\n", head->command);
    fprintf(file, "(returnjump%d)\n", rtn);
    rtn++;
}*/

void write_push(comm* head, FILE* file){
    char* arg1 = head->arg1;
    char* arg2 = head->arg2;
    fprintf(file, "\n//%s %s %s\n", head->command, head->arg1, head->arg2); //DEBUG
    if (strcmp(arg1, "temp") == 0 || strcmp(arg1, "static") == 0 || strcmp(arg1, "pointer") == 0){
        if (strcmp(arg1, "temp") == 0){
            get_temp_value(arg2, file);
        }
        else if(strcmp(arg1, "pointer") == 0){
            get_pointer_address(arg2, file);
            fprintf(file, "D=M\n");
        }
        else if (strcmp(arg1, "static") == 0){
            get_static_address(arg2, file);
        }
        fprintf(file, "D=M\n");
    }
    else if (strcmp(arg1, "constant") == 0){
        get_constant(arg2, file);
    }
    else{
        get_constant(arg2, file);
        get_address_value(arg1, file);
    }
    push(file);
}

void write_pop(comm* head, FILE* file){
    char* arg1 = head->arg1;
    char* arg2 = head->arg2;
    fprintf(file, "\n//%s %s %s\n", head->command, head->arg1, head->arg2); //DEBUG
    if (strcmp(arg1, "temp") == 0 || strcmp(arg1, "static") == 0 || strcmp(arg1, "pointer") == 0){
        pop(file);
        if (strcmp(arg1, "temp") == 0){
            get_temp_value(arg2, file);
        }
        else if(strcmp(arg1, "pointer") == 0){
            get_pointer_address(arg2, file);
        }
        else if (strcmp(arg1, "static") == 0){
            get_static_address(arg2, file);
        }
        fprintf(file, "M=D\n");
    }
    else{
        get_constant(arg2, file);
        store_address_index(arg1, file);
        pop(file);
        fprintf(file, "@R13\n");
        fprintf(file, "A=M\n");
        fprintf(file, "M=D\n");
    }
}

void get_pointer_address(char* arg2, FILE* file){
    fprintf(file, "%s\n", address_name_map[atoi(arg2)].value);
}

void get_static_address(char* arg2, FILE* file){
    static int addr = 16;
    int i = atoi(arg2);
    if (static_map[i] == 0){
        static_map[i] = addr;
        addr++;
    }
    fprintf(file, "@%d\n", static_map[i]);
}

void get_temp_value(char* value, FILE* file){
    for (int i = 0; i < 8; i++){
        if(strcmp(value, temp_map[i].key) == 0){
            fprintf(file, temp_map[i].value);
            fprintf(file, "\n");
        }
    }
}

void get_address_value(char* value, FILE* file){
    for(int i = 0; i < 4; i++){
        if(strcmp(value, address_name_map[i].key) == 0){
            fprintf(file, address_name_map[i].value);
            fprintf(file, "\n");
            fprintf(file, "A=D+M\n");
            fprintf(file, "D=M\n");
        }
    }
}

void store_address_index(char* value, FILE* file){
    for(int i = 0; i < 4; i++){
        if(strcmp(value, address_name_map[i].key) == 0){
            fprintf(file, address_name_map[i].value);
            fprintf(file, "\n");
            fprintf(file, "D=D+M\n");
            fprintf(file, "@R13\n");
            fprintf(file, "M=D\n");
        }
    }
}



void write_label(comm* head, FILE* file){}
void write_goto(comm* head, FILE* file){}
void write_if(comm* head, FILE* file){}
void write_function(comm* head, FILE* file){}
void write_return(comm* head, FILE* file){}
void write_call(comm* head, FILE* file){}

char* get_command_asm(char* line){
    for (int i = 0; i < 6; i++){
        if(strcmp(line, commands_map[i].key) == 0){
            return commands_map[i].value;
        }
    }
}