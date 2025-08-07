#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "code_writer.h"
#include "parser.h"

typedef struct{
    void* key;
    char* value;
}map;

static FILE* w_file;

const map address_name_map[4] = {{"this", "@THIS"}, {"that", "@THAT"}, {"local", "@LCL"}, {"argument", "@ARG"}};


void vm_translator(comm* head, char* dir){ 
    
    init(w_file, dir);
    while (head != NULL){
        translate_line(head, w_file);
        head = head->next;
    }
    end(w_file);
}

void init(FILE* file, char* dir){
    set_file(dir);
    bootstrap_init(file);
    //jump_init(file);
}

void set_file(char* dir){
    w_file = fopen(dir, "w");
}

FILE* get_file(){
    return w_file;
}

void asmprint(const char* fmt, ...){
    FILE* file = get_file();
    va_list args;
    for (va_start(args, fmt); *fmt != '\0'; ++fmt){
        switch(*fmt){
            case 'd':
                int d = va_arg(args, int);
                fprintf(w_file, "%d", d);
                break;
            case 's':
                char* s = va_arg(args, char*);
                fprintf(w_file, "%s", s);
                break;
            default:
                puts("Unknown formatter!");
                goto END;
        }
    }
END:
    va_end(args);
    fprintf(file, "\n");
}


void bootstrap_init(FILE* file){
    constant("256", file);
    fprintf(file, "@SP\n");
    fprintf(file, "M=D\n");
    fprintf(file, "@Sys.init\n");
    fprintf(file, "0;JMP\n");
}

void end(FILE* file){
    fprintf(file, "\n//End of Line\n");
    fprintf(file, "@program_end\n");
    fprintf(file, "(program_end)\n");
    fprintf(file, "0;JMP\n");
}

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
    printf("%s %s %s\n", head->command, head->arg1, head->arg2);
    switch (head->c_type){
        case (C_ARITHMETIC):
            write_arithmetic(head->command, file);
            break;
        case (C_JUMP):
            write_jump(head->command, file);
            break;
        case (C_PUSH):
            write_push(head->arg1, head->arg2, file);
            break;
        case (C_POP):
            write_pop(head->arg1, head->arg2, file);
            break;
        case (C_LABEL):
            write_label(head->arg1, file);
            break;
        case (C_GOTO):
            write_goto(head->arg1, file);
            break;
        case (C_IF):
            write_if(head->arg1, file);
            break;
        case (C_FUNCTION):
            write_function(head->arg1, head->arg2, file);
            break;
        case (C_RETURN):
            write_return(file);
            break;
        case (C_CALL):
            write_call(head->arg1, head->arg2, file);
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

void constant(char* value, FILE* file){
    fprintf(file, "@");
    fprintf(file, value);
    fprintf(file, "\n");
    fprintf(file, "D=A\n");
}

void write_arithmetic(char* command, FILE* file){
    fprintf(file, "\n//%s\n", command);  //DEBUG
    fprintf(file, "@SP\n");
    if (strcmp(command, "not") != 0 && strcmp(command, "neg") != 0){
        fprintf(file, "AM=M-1\n");
        fprintf(file, "D=M\n");
        fprintf(file, "A=A-1\n");
    }
    else{
        fprintf(file, "A=M-1\n");
    }
    fprintf(file, "MD=");
    fprintf(file, get_command_asm(command));
    fprintf(file, "\n");
}

void write_jump(char* command, FILE* file){
    char* jump;
    static int counter = 0;
    char* jump_map[] = {"gt", "JGT", "lt", "JLT", "eq", "JEQ"};
    
    
    for (int i = 0; i < 6; i += 2){
        if (strcmp(jump_map[i], command) == 0){
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

void write_push(char* arg1, char* arg2, FILE* file){;
    fprintf(file, "\n//Push %s %s\n", arg1, arg2); //DEBUG
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
        constant(arg2, file);
    }
    else{
        constant(arg2, file);
        get_address_value(arg1, file);
    }
    push(file);
}

void write_pop(char* arg1, char* arg2, FILE* file){
    fprintf(file, "\n//Pop %s %s\n", arg1, arg2); //DEBUG
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
        constant(arg2, file);
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
    static int static_map[240];
    static int addr = 16;
    int i = atoi(arg2);
    if (static_map[i] == 0){
        static_map[i] = addr;
        addr++;
    }
    fprintf(file, "@%d\n", static_map[i]);
}

void get_temp_value(char* value, FILE* file){
    map temp_map[8] = {{"0", "@R5"}, {"1", "@R6"}, {"2", "@R7"}, {"3", "@R8"}, {"4", "@R9"}, {"5", "@R10"}, {"6", "@R11"}, {"7", "@R12"}};

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



void write_label(char* arg1, FILE* file){
    fprintf(file, "\n//label %s\n", arg1);
    fprintf(file, "(%s)\n", arg1);
}

void write_goto(char* arg1, FILE* file){
    fprintf(file, "\n//goto %s\n", arg1);
    fprintf(file, "@%s\n", arg1);
    fprintf(file, "0;JMP\n");
}

void write_if(char* arg1, FILE* file){
    fprintf(file, "\n//if-goto %s\n", arg1);
    pop(file);
    fprintf(file, "@%s\n", arg1);
    fprintf(file, "D;JNE\n");
}

void write_function(char* arg1, char* arg2, FILE* file){
    fprintf(file, "\n//function %s %s\n", arg1, arg2);
    fprintf(file, "(%s)\n", arg1);
    zero_local(arg2, file);

}

void zero_local(char* arg2, FILE* file){
    int n = atoi(arg2);
    constant("0", file);
    for (int i = 0; i < n; i++){
        push(file);
    }
}

void write_return(FILE* file){
    char* map[] = {"@THAT", "1", "@THIS", "2", "ARG", "3", "LCL", "4"};

    fprintf(file, "\n//return\n");

    fprintf(file, "@LCL\n");
    fprintf(file, "D=M\n");
    fprintf(file, "@R14\n");
    fprintf(file, "M=D\n");

    constant("5", file);
    fprintf(file, "@R14\n");
    fprintf(file, "A=M-D\n");
    fprintf(file, "D=M\n");
    fprintf(file, "@R15\n");
    fprintf(file, "M=D\n");

    pop(file);
    fprintf(file, "@ARG\n");
    fprintf(file, "A=M\n");
    fprintf(file, "M=D\n");

    fprintf(file, "@ARG\n");
    fprintf(file, "D=M+1\n");
    fprintf(file, "@SP\n");
    fprintf(file, "M=D\n");

    for (int i = 0; i < 4; i +=2 ){
        constant(map[i+1], file);
        fprintf(file, "@R14\n");
        fprintf(file, "A=M-D\n");
        fprintf(file, "D=M\n");
        fprintf(file, "%s\n", map[i]);
        fprintf(file, "M=D\n");
    }

    fprintf(file, "@R15\n");
    fprintf(file, "A=M\n");
    fprintf(file, "0;JMP\n");
}

void write_call(char* arg1, char* arg2, FILE* file){
static int counter = 0;
    char* p[] = {"@LCL", "@ARG", "@THIS", "@THAT"}; 

    fprintf(file, "\n//call %s %s\n", arg1, arg2);
    fprintf(file, "@return%d\n", counter);
    fprintf(file, "D=A\n");
    push(file);
    for (int i = 0; i < 4; i++){
        fprintf(file, "%s\n", p[i]);
        fprintf(file, "D=M\n");
        push(file);
    }
    constant(arg2, file);
    fprintf(file, "@5\n");
    fprintf(file, "D=D+A\n");
    fprintf(file, "@SP\n");
    fprintf(file, "D=M-D\n");
    fprintf(file, "@ARG\n");
    fprintf(file, "M=D\n");
    fprintf(file, "@%s\n", arg1);
    fprintf(file, "0;JMP\n");
    fprintf(file, "(return%d)\n", counter);
    counter++;
    
}

char* get_command_asm(char* line){
    map commands_map[6] = { {"add", "D+M"}, {"sub", "M-D"}, {"and", "D&M"}, {"or", "D|M"}, {"not", "!D"}, {"neg", "-D"}};
    for (int i = 0; i < 6; i++){
        if(strcmp(line, commands_map[i].key) == 0){
            return commands_map[i].value;
        }
    }
}