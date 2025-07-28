#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "assembler.h"


//Struct to store each line of code until it can be printed

node_code* nc_head; //Node code head!
node_code* nc_tail; //Node code tail!

//Struct to store symbol name and it's address;


//Array to store symbol nodes.
symbol** symbols;

static int defined_addresses[24] = {0, 1, 2, 3, 4, 16384, 24576, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};
static char* defined_symbols[24] = {"SP", "LCL", "ARG", "THIS", "THAT", "SCREEN", "KBD", "R0", "R1", "R2", "R3", "R4", "R5", 
    "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", "0"};


//Handles the file location; input file for .asm, output file for .hack
static char input_file[256]; //.asm
static char output_file[256]; //.hack
static char* filename; //main file directory

//Current arrays to convert operands and jumps to their binary command
//Index value is equal to the ACSII value of the assembler command, stored int is the decimal value of binary command
//TODO create hash to reduce size
int operand_matrix[337]= {
     [48] = 42, [49] = 63, [65] = 48, [68] = 12, [78] = 112, [94] = 58, [98] = 49,
     [101] = 13, [110] = 51, [111] = 113, [113] = 15, [123] = 115, [157] = 55, [159] = 50, 
     [160] = 31, [162] = 14, [170] = 119, [171] = 0, [172] = 114, [176] = 2, [178] = 19, 
     [179] = 7, [184] = 64, [189] = 66, [191] = 83, [192] = 71, [336] = 21, [270] = 85
    };
int jump_matrix[61] = {[29] = 1, [24]=2, [14] = 3, [34] = 4, [21] = 5, [19] = 6, [31] = 7};

int main(void){
    //Once this program is finished, main can take an ARG with appropiate file name
    //Currently, just leaving the directory like this allows for quicker programming
    filename = "..\\project_files\\6\\pong\\Pong";
    int hash_size = 1009;
    init(hash_size);

    first_pass(hash_size);
    second_pass(hash_size);
    //closes out program
    exit_program(hash_size); 
    
    printf("Program Complete"); //Let's me know the program was successful
}

void init(int hash_size){
    set_file_exts();
    symbol** arr = calloc(hash_size, sizeof(char*));
    symbols = arr;
    init_predefined_symbols(hash_size);
}

void exit_program(int hash_size){
    destroy_node_code();
    destroy_symbol(hash_size);
    free(symbols);
}


//Function to concat file path with ext
void set_file_exts(){
    strcpy(input_file, filename);
    strcat(input_file, ".asm");

    strcpy(output_file, filename);
    strcat(output_file, ".hack");
}

//Opens the file to read; returns pointer
FILE* read_file(){
    FILE *fptr;
    fptr = fopen(input_file, "r");

    if (!fptr){
        printf("Error: Could not open file.\n");
        exit(1);
    }
    return fptr;
}

//Opens file to write; returns pointer
FILE* write_file(){
    FILE *fptr;
    fptr = fopen(output_file, "w");

    if (!fptr){
        printf("Error: Could not open file.\n");
        exit(1);
    }
    return fptr;
}

void first_pass(int hash_size){
    FILE* r_file = read_file();
    char* line = next_line(r_file);

    while (line != NULL){
        line = clean_line(line);
        seperate_code(line, hash_size);
        line = next_line(r_file);
    }
    fclose(r_file);
}

void seperate_code(char* line, int hash_size){
    static int line_num = 0; //Tracks line number to properly register symbols. 
    switch (line[0]) {
        case '@': //All A instructions start with @
            new_node_code(++line, 'A');
            line_num++;
            //printf(line);
            break;
        case 'A':
            //Pass through!
        case 'M':
            //Pass through!
        case 'D':
            //Pass through!
        case '0': //All C instructions start with either A, M, D, or 0
            new_node_code(line, 'C');
            line_num++;
            break;
        case '(': //Unregistered symbols. They need to be removed from the code and put into a register
            printf("%s\n", line);
            line[strlen(line)-1] = '\0'; //Removes right bracket
            ++line; //Removes left bracket
            printf("%s\n", line);
            label_symbols(line, hash_size, line_num);
        default:
            break;
        }
}


char* next_line(FILE* file){
    char buffer[1024];
    char* line = calloc(1024, sizeof(char));
    if (fgets(buffer, sizeof(buffer), file) != NULL){
        strcpy(line, buffer);
        return line;
    }
    free(line);
    return NULL;
}

char* clean_line(char* line){
    char* end;
    if (*line == 0){
        return line;
    }
    while(isspace(line[0])){
        line++;
    }

    end = line + strlen(line) -1;
    while(isspace(end[0])){
        end--;
    }
    end[1] = '\0';
    return line;
}

void new_node_code(char* data, char type){
    node_code* newNode = create_node_code(data, type);
    add_node_code(newNode);
}

node_code* create_node_code(char* data, char type){
    node_code* newNode = calloc(1, sizeof(node_code));
    newNode->data = data;
    switch (type){
        case 'A':
            newNode->fp = a_instruction;
            break;
        case 'C':
            newNode->fp = c_instruction;
    }
    return newNode;
}

void add_node_code(node_code* code){
    if (nc_head == NULL){
        nc_head = code;
        nc_tail = code;
    }
    else{
        nc_tail->next = code;
        nc_tail = code;
    }
}

void traverse_code(){
    node_code* nc_current = nc_head;
    while (nc_current != NULL){
        printf(nc_current->data);
        nc_current = nc_current -> next;
    }
}

void destroy_node_code(){
    node_code* current = nc_head;
    node_code* next;
    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }
}

static void init_predefined_symbols(int hash_size){
    for (int i = 0; i < 24; i++){
        symbol* sym = create_symbol(defined_symbols[i], defined_addresses[i]);
        add_symbol(sym, hash_size);
    }
}

static void label_symbols(char* label, int hash_size, int address){
    symbol* sym = create_symbol(label, address);
    add_symbol(sym, hash_size);
}

void add_symbol(symbol* sym, int hash_size){
    int hash = string_to_hash(sym->name, hash_size);
    symbol* curr = symbols[hash];
    if (symbols[hash] == NULL){
        symbols[hash] = sym;
    }
    else{
        while (curr->next != NULL){
            curr = curr->next;
        }
    curr->next = sym;}
}

symbol* create_symbol(char* line, int num){
    symbol* newSym = calloc(1, sizeof(symbol));
    newSym->name = line;
    newSym->address = num;
    newSym->next = NULL;
    return newSym;
}

symbol* create_var_symbol(char* line){
    static int var_num = 16;
    symbol* newSym = create_symbol(line, var_num);
    var_num++;
    return newSym;
}

int get_symbol_address(char* line, int hash_size){
    symbol* sym = find_symbol(line, hash_size);
    if (sym == NULL){
        return -1;
    }
    else{
        return sym->address;
    }
}

symbol* find_symbol(char* line, int hash_size){
    int hash = string_to_hash(line, hash_size);
    char* name;
    int compare;
    symbol* curr_sym;

    curr_sym = symbols[hash];
    while (curr_sym != NULL){
        name = curr_sym->name;
        compare = strcmp(name, line);
        if (compare == 0){
            return curr_sym;
        }
        curr_sym = curr_sym->next;
    }
    return NULL;
}

void traverse_symbols(int hash_size){
    for (int i = 0; i < hash_size; i++){
        printf("Array @%d", i);
        if(symbols[i] != NULL){
            symbol* curr = symbols[i];
            symbol* next;
            while(curr != NULL){
                printf(" -> %s: %d", curr->name, curr->address);
                next = curr->next;
                curr = next;
            }
        }
        else {
            printf(" -> %s", symbols[i]);
        }
        printf("\n");
    }
}

void destroy_symbol(int hash_size){
    for (int i = 0; i < hash_size; i++){
        if(symbols[i] != NULL){
            symbol* curr = symbols[i];
            symbol* next;
            while(curr != NULL){
                next = curr->next;
                free(curr);
                curr = next;
            }
        }
    }
}

void second_pass(int hash_size){
    int num;
    FILE* w_file;
    w_file = write_file();
    node_code* curr = nc_head;
    while (curr != NULL){
        num = curr->fp(curr->data, hash_size);
        print_to_bin(w_file, num);
        curr = curr->next;
        
    }
    fclose(w_file);
}

int a_instruction(char* line, int hash_size){
    int num;
    if(atoi(line) == 0){
        num = get_symbol_address(line, hash_size);
        if (num < 0){
            symbol* sym = create_var_symbol(line);
            add_symbol(sym, hash_size);
            num = sym->address;
        }
    }
    else{
        num = atoi(line);
    }
    return num;
}  

int c_instruction(char* line, int hash_size){
    int value = 57344; //Adds the non-changing range from 2^13->2^15 to total value to properly print to file
    char* args[3] = {};

    c_parse(line, args);
    value += c_register(args[0]);
    value += c_operand(args[1]);
    value += c_jump(args[2]);

    return value;
}

int c_parse(char* line, char** args){
    char* temp = line;
    while (temp[0] != '\0'){
        switch(temp[0]){
            case '=':
                args[0] = strtok(line, "=");
                args[1] = strtok(NULL, "=");
                args[2] = (".");
                return 0;
            case ';':
                args[0] = (".");
                args[1] = strtok(line, ";");
                args[2] = strtok(NULL, ";");
                return 0;
        }
        temp++;
    }
}


int c_jump(char* line){
    int value = line[0] + line[1] + line[2];
    return jump_matrix[value%100];
}

int c_operand(char* line){
    int value = 0;
    int temp = 0;

    if(line[0] == 'A' || line[0] == 'M'){
        temp += 1;
    }

    if(line[0] =='M' || line[1] == 'M' || line[2] == 'M'){
        value+=1;
    }
    while (line[0] != '\0' && line[0] != ';' && line[0] != '='){
        value += line[0];
        line++;
    }
    if (value == 178 || value == 191){
        value += temp;
    }
    value = operand_matrix[value];
    
    return value*64; //Operand matrix is in the 2^1 -> 2^7 binary value; multiplying by 64 (2^6) pushes it to the 2^7->2^13 range
}

int c_register(char* line){
    int value = 0;
    while (line[0] != '\0' && line[0] != ';' && line[0] != '='){
        switch (line[0]){
            case 'A':
                value += 32;
                break;
            case 'D':
                value += 16;
                break;
            case 'M':
                value += 8;
                break;
        }
        line++;
    }
    return value;
}

int print_to_bin(FILE* file, int dec){
    char binary[16];
    int num;
    for (int i = 15; i >= 0; i--){
        num = dec%2;
        dec /= 2;
        binary[i] = num + '0';
    }
    binary[16] = '\0';
    fprintf(file, "%s\n", binary);
}

//Adds all the ASCII values in a string to create a hash.
int string_to_hash(char* string, int size){
    int diff = 86; //Helps give a buffer between closely related chars. ex: ab, ba; ret10, ret11, ret11.
    unsigned int ascii = 1; //Numbers get large enough they swap over to negatives if not put as an unsigned int.
    char c;
    while(string[0] != '\0'){
        c = string[0];
        ascii = ((ascii*diff) + c);
        string++;
    }
    ascii = ascii*331%size;
    return ascii;
}