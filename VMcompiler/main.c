#include "parser.h"
#include "code_writer.h"

void main(){
    comm* head;
    char* dir = "..\\..\\project_files\\7\\BasicTest";
    char* dir2 = "..\\..\\project_files\\7\\BasicTest\\BasicTest.asm";
    head = parse_vm(dir);
    vm_translator(head, dir2);

}