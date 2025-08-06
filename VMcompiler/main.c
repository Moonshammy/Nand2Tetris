#include "parser.h"
#include "code_writer.h"

void main(){
    comm* head;
    char* dir = "..\\..\\project_files\\7\\StackTest";
    char* dir2 = "..\\..\\project_files\\7\\StackTest\\StackTest.asm";
    head = parse_vm(dir);
    vm_translator(head, dir2);

}