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
    
    init(dir);
    while (head != NULL){
        translate_line(head);
        head = head->next;
    }
    end();
}

void init(char* dir){
    set_file(dir);
    bootstrap_init();
    jump_init();
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


void bootstrap_init(){
    asmprint("s", "//Bootstrap");  //DEBUG
    constant("256");
    asmprint("s", "@SP");
    asmprint("s", "M=D");
    write_call("Sys.init", "0");
}

void end(){
    asmprint("s", "\n//End of Line");  //DEBUG
    asmprint("s", "@program_end");
    asmprint("s", "(program_end)");
    asmprint("s", "0;JMP");
}

void jump_init(){
    char* start[] = {"start_gt", "start_lt", "start_eq"};
    char* true[] = {"true_gt", "true_lt", "true_eq"};
    char* end[] = {"end_gt", "end_lt", "end_eq"};
    char* type[] = {"JGT", "JLT", "JEQ"};
    for (int i = 0; i < 3; i++){
        asmprint("ss", "\n//start of ", type[i]); //DEBUG
        asmprint("sss", "(", start[i], ")");
        asmprint("s", "@SP");
        asmprint("s", "AM=M-1");
        asmprint("s", "D=M");
        asmprint("s", "A=A-1");
        asmprint("s", "D=M-D");
        asmprint("ss", "@", true[i]);
        asmprint("ss", "D;", type[i]);
        asmprint("s", "D=0");
        asmprint("ss", "@", end[i]);
        asmprint("s", "0;JMP");
        asmprint("sss", "(", true[i], ")");
        asmprint("s", "D=-1");
        asmprint("sss", "(", end[i], ")");
        asmprint("s", "@SP");
        asmprint("s", "A=M-1");
        asmprint("s", "M=D");
        asmprint("s", "@R13");
        asmprint("s", "A=M");
        asmprint("s", "0;JMP");
    }
}

void translate_line(comm* head){
    printf("%s %s %s\n", head->command, head->arg1, head->arg2);
    static char* curr_func;
    switch (head->c_type){
        case (C_ARITHMETIC):
            write_arithmetic(head->command);
            break;
        case (C_JUMP):
            write_jump(head->command);
            break;
        case (C_PUSH):
            write_push(head->arg1, head->arg2, curr_func);
            break;
        case (C_POP):
            write_pop(head->arg1, head->arg2, curr_func);
            break;
        case (C_LABEL):
            write_label(head->arg1);
            break;
        case (C_GOTO):
            write_goto(head->arg1);
            break;
        case (C_IF):
            write_if(head->arg1);
            break;
        case (C_FUNCTION):
            curr_func = head->arg1;
            write_function(head->arg1, head->arg2);
            break;
        case (C_RETURN):
            write_return();
            break;
        case (C_CALL):
            write_call(head->arg1, head->arg2);
            break;
    }
}

void push(){
    asmprint("s", "@SP"); 
    asmprint("s", "AM=M+1");
    asmprint("s", "A=A-1");
    asmprint("s", "M=D");
}
void pop(){
    asmprint("s", "@SP");
    asmprint("s", "AM=M-1");
    asmprint("s", "D=M");
}

void constant(char* value){
    asmprint("ss", "@", value);
    asmprint("s", "D=A");
}

void write_arithmetic(char* command){
    asmprint("ss", "\n//", command);  //DEBUG
    asmprint("s", "@SP");
    if (strcmp(command, "not") != 0 && strcmp(command, "neg") != 0){
        asmprint("s", "AM=M-1");
        asmprint("s", "D=M");
        asmprint("s", "A=A-1");
    }
    else{
        asmprint("s", "A=M-1");
    }
    asmprint("ss", "MD=", get_command_asm(command));
}

void write_jump(char* command){
    static int rtn = 0;
    char* jump;
    char* jump_map[] = {"gt", "JGT", "lt", "JLT", "eq", "JEQ"};
    for (int i = 0; i < 6; i += 2){
        if (strcmp(jump_map[i], command) == 0){
            jump = jump_map[i+1];
        }
    }
    asmprint("ss", "\n//Start of jump ", jump); //DEBUG
    asmprint("sd", "@returnjump", rtn);
    asmprint("s", "D=A");
    asmprint("s", "@R13");
    asmprint("s", "M=D");
    asmprint("ss", "@start_", command);
    asmprint("s", "0;JMP");
    asmprint("sds", "(returnjump", rtn, ")");
    rtn++;
}

void write_push(char* arg1, char* arg2, char* curr_func){;
    asmprint("ssss", "\n//Push ", arg1, " ", arg2); //DEBUG
    if (strcmp(arg1, "temp") == 0 || strcmp(arg1, "static") == 0 || strcmp(arg1, "pointer") == 0){
        if (strcmp(arg1, "temp") == 0){
            get_temp_value(arg2);
        }
        else if(strcmp(arg1, "pointer") == 0){
            get_pointer_address(arg2);
            asmprint("s", "D=M");
        }
        else if (strcmp(arg1, "static") == 0){
            get_static_address(arg2, curr_func);
        }
        asmprint("s", "D=M");
    }
    else if (strcmp(arg1, "constant") == 0){
        constant(arg2);
    }
    else{
        constant(arg2);
        get_address_value(arg1);
    }
    push();
}

void write_pop(char* arg1, char* arg2, char* curr_func){
    asmprint("ssss", "\n//Pop ", arg1, " ", arg2); //DEBUG
    if (strcmp(arg1, "temp") == 0 || strcmp(arg1, "static") == 0 || strcmp(arg1, "pointer") == 0){
        pop();
        if (strcmp(arg1, "temp") == 0){
            get_temp_value(arg2);
        }
        else if(strcmp(arg1, "pointer") == 0){
            get_pointer_address(arg2);
        }
        else if (strcmp(arg1, "static") == 0){
            get_static_address(arg2, curr_func);
        }
        asmprint("s", "M=D");
    }
    else{
        constant(arg2);
        store_address_index(arg1);
        pop();
        asmprint("s", "@R13");
        asmprint("s", "A=M");
        asmprint("s", "M=D");
    }
}

void get_pointer_address(char* arg2){
    asmprint("s", address_name_map[atoi(arg2)].value);
}

void get_static_address(char* arg2, char* curr_func){
    static char* var_map[256];
    static int address = 0;

    char* func;
    func = static_name(arg2, curr_func);
    
    
    for (int i = 0; i < address; i++){
        if (strcmp(func, var_map[i]) == 0){
            printf("%s\n\n", func);
            asmprint("sd", "@", i+16);
            goto END;
        }
    }

    var_map[address] = func;
    asmprint("sd", "@", address+16);
    address++;
END:
}

char* static_name(char* num, char* func){
    char* str = func;
    while (str[0] != '.' && str[0] != '\0'){
        str++;
    }
    func[strlen(func) - strlen(str)] = '\0';
    
    int len = strlen(func) + 1;
    char* string = calloc(len, sizeof(char));

    for (int i = 0; i < strlen(func); i++){
        string[i] = func[i];
    }
    string[len-1] = num[0];

    return string;

}

void get_temp_value(char* value){
    map temp_map[8] = {{"0", "@R5"}, {"1", "@R6"}, {"2", "@R7"}, {"3", "@R8"}, {"4", "@R9"}, {"5", "@R10"}, {"6", "@R11"}, {"7", "@R12"}};

    for (int i = 0; i < 8; i++){
        if(strcmp(value, temp_map[i].key) == 0){
            asmprint("s", temp_map[i].value);
        }
    }
}

void get_address_value(char* value){
    for(int i = 0; i < 4; i++){
        if(strcmp(value, address_name_map[i].key) == 0){
            asmprint("s", address_name_map[i].value);
            asmprint("s", "A=D+M");
            asmprint("s", "D=M");
        }
    }
}

void store_address_index(char* value){
    for(int i = 0; i < 4; i++){
        if(strcmp(value, address_name_map[i].key) == 0){
            asmprint("s", address_name_map[i].value);
            asmprint("s", "D=D+M");
            asmprint("s", "@R13");
            asmprint("s", "M=D");
        }
    }
}



void write_label(char* arg1){
    asmprint("ss", "\n//label ", arg1); //DEBUG
    asmprint("sss", "(", arg1, ")");
}

void write_goto(char* arg1){
    asmprint("ss", "\n//goto ", arg1); //DEBUG
    asmprint("ss", "@", arg1);
    asmprint("s", "0;JMP");
}

void write_if(char* arg1){
    asmprint("ss", "\n//if-goto ", arg1); //DEBUG
    pop();
    asmprint("ss", "@", arg1);
    asmprint("s", "D;JNE");
}

void write_function(char* arg1, char* arg2){
    asmprint("ssss", "\n//function ", arg1, " ", arg2);  //DEBUG
    asmprint("sss", "(", arg1, ")");
    zero_local(arg2);

}

void zero_local(char* arg2){
    int n = atoi(arg2);
    constant("0");
    for (int i = 0; i < n; i++){
        push();
    }
}

void write_return(){
    char* map[] = {"@THAT", "1", "@THIS", "2", "@ARG", "3", "@LCL", "4"};

    asmprint("s", "\n//return"); //DEBUG

    //Frame(R14) = LCL
    asmprint("s", "@LCL");
    asmprint("s", "D=M");
    asmprint("s", "@R14");
    asmprint("s", "M=D");

    //RTN = frame-5
    constant("5");
    asmprint("s", "@R14");
    asmprint("s", "A=M-D");
    asmprint("s", "D=M");
    asmprint("s", "@R15");
    asmprint("s", "M=D");

    //*ARG = pop(). Puts return value at ARG
    pop();
    asmprint("s", "@ARG");
    asmprint("s", "A=M");
    asmprint("s", "M=D");

    //Restores SP to caller
    asmprint("s", "@ARG");
    asmprint("s", "D=M+1");
    asmprint("s", "@SP");
    asmprint("s", "M=D");

    for (int i = 0; i < 8; i +=2 ){
        constant(map[i+1]);
        asmprint("s", "@R14");
        asmprint("s", "A=M-D");
        asmprint("s", "D=M");
        asmprint("s",  map[i]);
        asmprint("s", "M=D");
    }

    asmprint("s", "@R15");
    asmprint("s", "A=M");
    asmprint("s", "0;JMP");
}

void write_call(char* arg1, char* arg2){
static int counter = 0;
    char* p[] = {"@LCL", "@ARG", "@THIS", "@THAT"}; 

    asmprint("ssss", "\n//call ", arg1, " ", arg2); //DEBUG

    //Push return address to stack
    asmprint("sd", "@return", counter);
    asmprint("s", "D=A");
    push();

    //Push LCL, ARG, THIS, THAT to stack
    for (int i = 0; i < 4; i++){
        asmprint("s", p[i]);
        asmprint("s", "D=M");
        push();
    }

    //Set ARG to SP-n-5; where n is number of args (arg2).
    constant(arg2);
    asmprint("s", "@5");
    asmprint("s", "D=D+A");
    asmprint("s", "@SP");
    asmprint("s", "D=M-D");
    asmprint("s", "@ARG");
    asmprint("s", "M=D");

    //Sets LCL = SP
    asmprint("s", "@SP");
    asmprint("s", "D=M");
    asmprint("s", "@LCL");
    asmprint("s", "M=D");

    //Jump to call
    asmprint("ss", "@", arg1);
    asmprint("s", "0;JMP");
    asmprint("sds", "(return", counter, ")");
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