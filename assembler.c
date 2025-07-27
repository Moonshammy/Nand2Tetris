#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"


//Struct to store each line of code until it can be printed
typedef struct node {
    char* data; //Should only store either A or C-instruction code (D=D+M or @15)
    int (*fp)(char*, int); //Points the function for whatever instruction is stored here
    struct node* next;  //Points to next code
}node_code;

node_code* nc_head; //Node code head!
node_code* nc_tail; //Node code tail!

//Struct to store symbol name and it's address;


//Array to store symbol nodes.
symbol** symbols;

//Handles the file location; input file for .asm, output file for .hack
char input_file[256]; //.asm
char output_file[256]; //.hack
char* filename; //main file directory
FILE* r_file;
FILE* w_file;



//Current arrays to convert operands and jumps to their binary command
//Index value is equal to the ACSII value of the assembler command, stored int is the decimal value of binary command
//TODO create hash to reduce size
int operand_matrix[321]= {
     [48] = 42, [49] = 63, [65] = 48, [68] = 12, [78] = 112, [94] = 58, [98] = 49,
     [101] = 13, [110] = 51, [111] = 113, [113] = 15, [123] = 115, [157] = 55, [159] = 50, 
     [160] = 31, [162] = 14, [160] = 119, [171] = 0, [172] = 114, [176] = 2, [178] = 19, 
     [179] = 7, [183] = 64, [189] = 66, [191] = 83, [192] = 71, [307] = 21, [320] = 85
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
    w_file = write_file();

    symbol** arr = calloc(hash_size, sizeof(char*));
    symbols = arr;
    init_predefined_symbols(hash_size);
}

void exit_program(int hash_size){

    destroy_node_code();
    destroy_symbol(hash_size);
    fclose(w_file);
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
    r_file = read_file();
    char* line = next_line();
    int line_num = 0; //Tracks line number to properly register symbols. 
    int num_syms = 0;
    while (line != NULL){
        line[strlen(line)-1] = '\0';
        switch (line[0]) {
            case '@': //All A instructions start with @
                new_node_code(++line, 'A');
                line_num++;
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
                line[strlen(line)-1] = '\0'; //Removes right bracket
                ++line; //Removes left bracket
                label_symbols(line, hash_size, line_num);
            default:
                break;
        }
        line = next_line();
    }
    fclose(r_file);
}

char* next_line(){
    int size = 100;
    char* buffer = (char*) malloc(size);
    if(fgets(buffer, size, r_file) != NULL){
        return buffer;
    }
    free(buffer);
    return NULL;
}

void new_node_code(char* data, char type){
    node_code* newNode = (node_code*)malloc(sizeof(node_code));
    newNode->data = data;
    switch (type){
        case 'A':
            newNode->fp = a_instruction;
            break;
        case 'C':
            newNode->fp = c_instruction;
    }

    if (nc_head == NULL){
        nc_head = newNode;
        nc_tail = newNode;
    }
    else{
        nc_tail->next = newNode;
        nc_tail = newNode;
    }
}

void transverse_code(){
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
    char* symbols[24] = {"SP", "LCL", "ARG", "THIS", "THAT", "SCREEN", "KBD", 
                        "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8",
                        "R9", "R10", "R11", "R12", "R13", "R14", "R15", "0"};
    int address[24] = {0, 1, 2, 3, 4, 16384, 24576, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};
    for (int i = 0; i < 24; i++){
        symbol* sym = create_symbol(symbols[i], address[i]);
        add_symbol(sym, hash_size);
    }
}

static void label_symbols(char* label, int hash_size, int address){
    symbol* sym = create_symbol(label, address);
    add_symbol(sym, hash_size);
}

int variable_symbols(){
    return -1;
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
    w_file = write_file();
    node_code* curr = nc_head;
    while (curr != NULL){
        num = curr->fp(curr->data, hash_size);
        print_to_bin(num);
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
    int value = 57344;
    if (line[1] == ';'){
        print_to_bin(value);
    }
}


int c_jump(char* line){
    return 0;
}

int c_operand(char* line){
    return 0;
}

int c_register(char* line){
    int value = 0;
    while (line[0] != '\0'){
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




int print_to_bin(int dec){
    char binary[16];
    int num;
    for (int i = 15; i >= 0; i--){
        num = dec%2;
        dec /= 2;
        binary[i] = num + '0';
    }
    binary[16] = '\0';
    fprintf(w_file, "%s\n", binary);
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

/*int string_to_hash(char* string, int size){
    int diff = 66; //Helps give a buffer between closely related chars. ex: ab, ba; ret10, ret11, ret11.
    unsigned int ascii = 1; //Numbers get large enough they swap over to negatives if not put as an unsigned int.
    char c;
    while(string[0] != '\0'){
        c = string[0];
        ascii = ((ascii*diff) + c)%size;
        string++;
    }
    return ascii;
}*/

//LEGACY; KEPT ONLY TO REFACTOR
/*
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


int convert_to_bin( int dec){
    int binary = 0;
    int counter = 1;
    while (dec != 0){
        int result = dec % 2;
        dec /= 2;
        binary += result * counter;
        counter *= 10;
    }
    printf("%d", binary);
    return binary;
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



*/