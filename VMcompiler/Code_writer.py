#from Assembler import hackToAsm

ROM = []
operators = {'add': 'D+M', 'sub': 'M-D', 'and': 'D&M', 'or': 'D|M', 'not': '!D', 'neg': '-D'}
bool_symbols = {'eq': 'JEQ', 'lt': 'JLT', 'gt': 'JGT'}

indexed_pointers = {'local': 'LCL', 'argument': 'ARG', 'this': 'THIS', 'that': 'THAT'}
equality_counter = 0
return_counter = 0


def code_writer(code, directory, program):
    program = f'{directory}\\{program}'

    write_init()
    
    #Decides what function to call dependent on which command
    for line in code:
        line = line.split()
        commands[line[0]](line)

    #Writing bits to make .asm and .hack files
    write_to_file(program)
    #hackToAsm(program)
    
#Writes .asm file line by line
def write_to_file(program):

    writer = open(program+'.asm', 'w')
    for line in ROM:
        writer.write(line + '\n')

#C_ARITHMETIC commands are passed here
def write_arithmetic(line):
    line = line[1]
    if line in bool_symbols:
        bool_test(line)
    else:
        operation(line)

#Helper to do operator translation
def operation(line):
    ROM.append('@SP')
    if line != 'not' and line != 'neg':
        ROM.append('AM=M-1')
        ROM.append('D=M')
        ROM.append('A=A-1')
    else:
        ROM.append('A=M-1')
    ROM.append(f'DM={operators[line]}')

#Helper to do jump commands
def bool_test(line):
    global equality_counter
    operation('sub')
    pop() #Get top value on stack

    ROM.append(f'@true{equality_counter}')
    ROM.append(f'D;{bool_symbols[line]}') #If true, jumps to (true) to set value to D
    ROM.append('D=0') #If false, no jump occurs, setting D to false (0)
    ROM.append(f'@end{equality_counter}') 
    ROM.append('0;JMP') #Force jumps false result to push function
    ROM.append(f'(true{equality_counter})') #If equality check is true, first jump comes here
    ROM.append('D=-1') #Sets D value to true (-1)
    ROM.append(f'(end{equality_counter})') #End of equality check
    
    push() #Pushes T/F value onto stack
    equality_counter += 1 #Increments amount of equality checks


def write_push_pop(line):
    command = line[0]; arg1 = line[1]; arg2 = line[2]
    if command == 'C_PUSH':
        push_command(arg1, arg2)
    else:
        pop_command(arg1, arg2)

def push_command(arg1, arg2):
    if arg1 == 'constant':
        constant(arg2)
        push()
    elif arg1 in indexed_pointers:
        set_pointer_index(arg1, arg2)
        ROM.append('@addr')
        ROM.append('A=M')
        ROM.append('D=M')
        push()
    else:
        goto_address(arg1, arg2)
        ROM.append('D=M')
        push()

def pop_command(arg1, arg2):
    if arg1 in indexed_pointers:
        set_pointer_index(arg1, arg2)
        pop()
        ROM.append('@addr')
        ROM.append('A=M')
        ROM.append('M=D')
    else:
        pop()
        goto_address(arg1, arg2)
        ROM.append('M=D')

#Set pointer(i) at @addr
def set_pointer_index(pointer, index):
    constant(index)
    ROM.append(f'@{indexed_pointers[pointer]}')
    ROM.append('D=D+M')
    ROM.append('@addr')
    ROM.append('M=D')

#Puts address either at this, that, or temp
def goto_address(arg1, arg2):
    if arg1 == 'temp':
        index = int(arg2) + 5
        address = 'R' + str(index)
        ROM.append(f'@{address}')
    elif arg1 == 'pointer':
        if arg2 == '0':
            ROM.append('@THIS')
        else:
            ROM.append('@THAT')
    else:
        ROM.append(f'@{arg1}{arg2}')

def init():
    write_init()
    bool_init()


def write_init():
    constant('256')
    ROM.append('@SP')
    ROM.append('M=D')
    write_call(['C_CALL', 'Sys.init', '0'])

def bool_init():
    tests = ['JEQ','JGT','JLT']
    for test in tests:
        bool_type = test
        ROM.append(f'(START_{bool_type})')
        ROM.append('@R15')
        ROM.append('D=M')
        ROM.append('A=A-1')
        ROM.append('D=M-D')
        ROM.append('M=0')
        ROM.append(f'@END_{bool_type}')
        ROM.append(f'D;{bool_type}')
        ROM.append('@SP')
        ROM.append('A=M-1')
        ROM.append('M=-1')
        ROM.append(f'(END_{bool_type})')
        ROM.append('@R15')
        ROM.append('A=M')
        ROM.append('0;JMP')

def write_label(label):
    ROM.append(f'({label[1]})')

def write_goto(label):
    ROM.append(f'@{label[1]}')
    ROM.append('0;JMP')

def write_if(label):
    pop()
    ROM.append(f'@{label[1]}')
    ROM.append('D;JNE')

def write_call(line):
    global return_counter
    func = line[1]
    args = line[2]
    
    ROM.append(f'@return{return_counter}')
    ROM.append('D=A')
    push()

    pushes = ['@LCL', '@ARG', '@THIS', '@THAT']
    for p in pushes:
        ROM.append(p)
        ROM.append('D=M')
        push()

    constant(args)
    ROM.append('@5')
    ROM.append('D=D+A')
    ROM.append('@SP')
    ROM.append('D=M-D')
    ROM.append('@ARG')
    ROM.append('M=D')

    ROM.append('@SP') 
    ROM.append('D=M')
    ROM.append('@LCL')
    ROM.append('M=D')

    ROM.append(f'@{func}')
    ROM.append('0;JMP')

    ROM.append(f'(return{return_counter})')

    return_counter += 1


def write_return(line):
    #Local in temp var FRAME
    ROM.append('@LCL')
    ROM.append('D=M')
    ROM.append('@FRAME')
    ROM.append('M=D')

    #Return address in temp var RET
    constant('5')
    ROM.append('@FRAME')
    ROM.append('A=M-D')
    ROM.append('D=M')
    ROM.append('@RET')
    ROM.append('M=D')

    #Put return value into stack
    pop()
    ROM.append('@ARG')
    ROM.append('A=M')
    ROM.append('M=D')

    #Restore SP to caller
    ROM.append('@ARG')
    ROM.append('D=M+1')
    ROM.append('@SP')
    ROM.append('M=D')

    #Restore THAT to caller
    constant('1')
    ROM.append('@FRAME')
    ROM.append('A=M-D')
    ROM.append('D=M')
    ROM.append('@THAT')
    ROM.append('M=D')

    #Restore THAT to caller
    constant('2')
    ROM.append('@FRAME')
    ROM.append('A=M-D')
    ROM.append('D=M')
    ROM.append('@THIS')
    ROM.append('M=D')

    #Restore ARG to caller
    constant('3')
    ROM.append('@FRAME')
    ROM.append('A=M-D')
    ROM.append('D=M')
    ROM.append('@ARG')
    ROM.append('M=D')

    #Restore LCL to caller
    constant('4')
    ROM.append('@FRAME')
    ROM.append('A=M-D')
    ROM.append('D=M')
    ROM.append('@LCL')
    ROM.append('M=D')

    #Return
    ROM.append('@RET')
    ROM.append('A=M')
    ROM.append('0;JMP')



    #Restore
def write_function(line):
    func = line[1]
    num_args = line[2]
    ROM.append(f'({func})')
    zero_local(int(num_args))

def zero_local(index):
    constant('0')
    for i in range(index):
        push()

#Makes removes latest stack value and puts it into D register; decrements pointer
def pop():
    ROM.append('@SP') #Go to Stack Pointer
    ROM.append('AM=M-1') #Both decrement pointer as well as go to decremented pointer address
    ROM.append('D=M') #Puts value into D register

#Puts value in D register at top of stack; increments pointer
def push():
    ROM.append('@SP') #Go to Stack Pointer
    ROM.append('M=M+1') #Increment Pointer, saves a step
    ROM.append('A=M-1') #Puts current address to pointer location before we incremented
    ROM.append('M=D') #Puts D register value onto stack

def constant(arg2):
    ROM.append(f'@{arg2}')
    ROM.append('D=A')

commands = {'C_ARITHMETIC': write_arithmetic, 'C_PUSH': write_push_pop,'C_POP': write_push_pop, 
            'C_LABEL': write_label, 'C_GOTO': write_goto, 'C_IF': write_if, 
            'C_FUNCTION': write_function, 'C_RETURN': write_return, 'C_CALL': write_call}