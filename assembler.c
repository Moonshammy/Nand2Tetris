#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"


//Not used currently,
enum intstruction{
    A_INSTRUCTION,
    B_INSTRUCTION
};

typedef struct{

    char* code;
    int value;


} code_node;

char input_file[256];
char output_file[256];
char* filename;

int operand_matrix[321]= {
     [48] = 42, [49] = 63, [65] = 48, [68] = 12, [78] = 112, [94] = 58, [98] = 49,
     [101] = 13, [110] = 51, [111] = 113, [113] = 15, [123] = 115, [157] = 55, [159] = 50, 
     [160] = 31, [162] = 14, [160] = 119, [171] = 0, [172] = 114, [176] = 2, [178] = 19, 
     [179] = 7, [183] = 64, [189] = 66, [191] = 83, [192] = 71, [307] = 21, [320] = 85
    };

int jump_matrix[61] = {[55] = 1, [50]=10, [40] = 11, [60] = 100, [47] = 101, [45] = 110, [57] = 111};

int address_table[32768]={};

int code_len;
int i;

int main(void){
    //TODO; create an init func
    //TODO; create an exit func
    filename = "..\\project_files\\6\\pong\\PongL";
    set_file_exts(filename);

    FILE* r_file = read_file();


    FILE* w_file = write_file();
    char* code[32768];
    

    for (i = 0; i < 32768; i++){
        code[i] = calloc(1, sizeof(char));
        if (code[i] == NULL) {
            perror("Memory allocation failed");
            return 1;
        }
    }

    code_len = first_pass(r_file, code);
    second_pass(w_file, code);

    fclose(r_file);
    fclose(w_file);
    for (i = 0; i < 32767; i++){
        free(code[i]);
    }

    printf("Program Complete");
}

void set_file_exts(char* filename){
    strcpy(input_file, filename);
    strcat(input_file, ".asm");

    strcpy(output_file, filename);
    strcat(output_file, ".hack");
}


FILE* read_file(){
    FILE *fptr;
    fptr = fopen(input_file, "r");

    if (!fptr){
        printf("Error: Could not open file.\n");
        exit(1);
    }
    return fptr;
}

FILE* write_file(){
    FILE *fptr;
    fptr = fopen(output_file, "w");

    if (!fptr){
        printf("Error: Could not open file.\n");
        exit(1);
    }
    return fptr;
}

int first_pass(FILE* file, char* code[]){
    int i = 0;
    char* line = next_line(file);
    while (line != NULL){
        strtok(line, " ");
        if (strcmp(line, "//") && strcmp(line, " ") && strcmp(line, "\n")){
            memcpy(code[i], line, strlen(line)+1);
            i++;
        }
        line = next_line(file);
    }
    return i;
}

char* next_line(FILE* file){
    char buffer[1024];
    char* str;
    str = fgets(buffer, sizeof(buffer), file);
        if (str != NULL){
            return str;
        }
    return NULL;
}

int second_pass(FILE* file, char* line[]){
    for (int i = 0; i < code_len; i++){
        char* code = line[i];
        if (code[0] == '@'){
             a_instruction(file, code+1);
        }
        else{
            c_instruction(file, code);
        }
    }
}

void a_instruction(FILE* file, char* line){
    int binary = 0;
    int num = atoi(line);
    int counter = 1;
    while (num != 0){
        int result = num % 2;
        num /= 2;
        binary += result * counter;
        counter *= 10;
    }
    fprintf(file, "%016d\n", binary);
}

void c_instruction(FILE* file, char* line){
    char* arg1 = calloc(5, sizeof(char));
    char* arg2 = calloc(5, sizeof(char));
    char sym = ' ';
    char a;

    int i = 0;
    while(*(line+1) != '\0'){
        if(*line == '=' || *line == ';'){
            sym = *line;
            i = 0;
        }
        else{
            a = *line;
            switch (sym) {
                case '=':
                    //Pass through
                case ';':
                    arg2[i] = a;
                    i++;
                    break;
                default:
                    arg1[i] = a;
                    i++;
                    break;
            }
        }
        line++;
    }

    switch (sym){
        case '=':
            fprintf(file, "%03d%07d%03d%03d\n", 111, c_operand(arg2), c_register(arg1), 0);
            break;
        case ';':
            fprintf(file, "%03d%07d%03d%03d\n", 111, c_operand(arg1), 0, c_jump(arg2)); 
            break;
    }
    
    free(arg1);
    free(arg2);
}

int c_register(char* reg){
    int bin = 0;
    while (*(reg) != '\0'){
        if (*reg == 'A'){
            bin += 100;
        }
        else if (*reg == 'D'){
            bin += 10;
        }
        else if (*reg == 'M'){
            bin += 1;
        }
        reg++;
    }
    return bin;
}

int c_operand(char* operand){
    char a = ' ';
    int ascii = 0;
    int temp = 0;
    if (operand[0] == '\0'){
        return 0;
    }
    
    //This is for the two cases where the pairs {D-M, M-D} and {D-A, A-D} have the exact same value;
    //Increments the total value by one if A or M are the leading chars when 190 and 178 are met.
    if(operand[0] == 'A' || operand[0] == 'M'){
        temp += 1;
    }

    //For some reason, there's two values where the ASCII's added up equal another ASCII pair, breaking the hashtable
    //This just adjusts all operations with M to increase the ASCII value by one.
    if(operand[0] == 'M' || operand[2] == 'M'){
        ascii += 1;
    }

    while (*operand != '\0'){
        a = *operand;
        ascii += a;
        operand++;
    }
    //Adds 1 to ascii value if A or M are leading values when ASCII is 178 or 190
    //if D is leading value, 0 is added
    if (ascii == 178 || ascii == 190){
        ascii += temp;
    }

    return convert_to_bin(operand_matrix[ascii]);
}
    
int c_jump(char* jump){
    if (jump[0] == '\0' ){
        return 0;
    }
    int ascii = 0;
    for (int i = 1; i < 3; i++){
        int a = jump[i];
        ascii += a;
    }
    ascii = ascii % 100; //ASCII values all in hundreds place. Reduces required array size down
    return jump_matrix[ascii];
}

int convert_to_bin(int dec){
    int binary = 0;
    int counter = 1;
    while (dec != 0){
        int result = dec % 2;
        dec /= 2;
        binary += result * counter;
        counter *= 10;
    }
    return binary;
}
