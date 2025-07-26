# Nand2Tetris
Compilers for Nand2Tetris course

These programs are part of the Nand2Tetris Part 2 course. They consist of program that reads an assembly language. A program that reads a "VM".
Finally a program that reads the "Jack" language and compiles all down to the machine language level to work in the Nand2Tetris IDE.

I created the Python programs up to the "VM" level initially, eventually deciding that I'd like to restart from C. 
The goal is to learn C and tighten up how I handle the programming languages. This README primarily deals only with the C programs.

Assembler:
The assembly language is a (near) 1 to 1 conversion to binary code/machine language of the "Hack" program.
The goal of this program is to translate the assembler code into machine code. 
There are three primary things that we look for:
    A. A (address) instructions, all start with the @ symbol.
    B. C (computation) instructions, all start with either A, M, D, or 0.
    C. Label Symbols. They are in brackets as so "(symbol)".
    D. All other things such as comment (//) are ignored.

A instructions come in a few varities. 
    They are constants "@256", where the numerical value is the exact location we are going.
    Then there are predefined Symbols, @SP, @LCL, @R12...
    Label Symbols (label.symbol) -> @label.symbol -> @(address of label) ->@1765
        -To note, these do not get translated into machine language. They get stored into some sort of table.
    Variable Symbols @var -> @16+N; where N is the number of variable symbols.

C only need to be parsed into three types of computation commands.
    comp: instructs CPU to conduct a specific operation
    dest: where the value from the command will go
    jump: if value is true to jump statement, jump ROM address indicated by current A RAM address

The first step the program takes is going line by line through the file and storing all A and C instructions into a linked list.
The linked list gives the ability to keep the code on standby while label symbols are all put into a table. The linked list also
adds a function pointer to either the a_instruction() or c_instruction, meaning aside from traversal, no additional steps are required.

Additionally, all label symbols are hashed and put into a char pointer array. 
    //Note, making an array of structs to store label symbols might be better

The second step is going through and translating all A and C instructions. The inital code I wrote could handle this fine without label and variable symbols.
Currently this part is not fleshed out yet.