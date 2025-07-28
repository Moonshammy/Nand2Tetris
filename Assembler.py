next_address = 16

address_table = {"SP": 0, "LCL": 1, "ARG": 2, "THIS": 3, "THAT": 4, 
                 "R0": 0, "R1": 1, "R2": 2, "R3": 3, "R4": 4, "R5": 5, 
                 "R6": 6, "R7": 7, "R8": 8, "R9": 9, "R10": 10, "R11": 11, 
                 "R12": 12, "R13": 13, "R14": 14, "R15": 15, 
                 "SCREEN": 16384, "KBD": 24576}

comp_table = {"0":42, "1":63, "-1":58, "D":12, "A":48, "!D": 13, "!A": 49, "-D": 15, "-A": 51,
              "D+1": 31, "A+1": 55, "D-1": 14, "A-1": 50, "D+A": 2, "D-A": 19, "A-D": 7, "D&A": 0, "D|A": 21,
              "M": 112, "!M": 113, "-M": 115, "M+1": 119, "M-1": 114, "D+M": 66, "D-M": 83, "M-D": 71,
              "D&M": 64, "D|M": 85}

jump_table = {"null": 0, "JGT": 1, "JEQ": 2, "JGE": 3, "JLT": 4, "JNE": 5, "JLE": 6, "JMP": 7}

binary_code = []

def hackToAsm(file):
    reader = open(file + ".asm")
    #First pass
    code = first_pass(reader)
    reader.close()
    
    for line in code:
        second_pass(line)

    with open(file +".hack", 'w') as file:
        for line in binary_code:
            line += "\n"
            file.write(line)

def first_pass(file):
    counter = 0
    code = []
    for line in file:
        line = line.strip()
        if line == '\n' or line[0:2] == "//" or line == '':
            continue
        elif line[0] == '(':
            address_table[line[1:-1]] = counter #Adds location of ROM address to table
        else:
            code.append(line)
            counter += 1 #Keeps track of what line the code is on
    return code

def second_pass(code):
    char = code[0]
    if char == '@':
        binary_code.append(a_instruction(code[1:]))
    elif char == "D" or char == "M" or char == "A" or char =="0":
        c_instruction(code)

def a_instruction(address):
    global next_address
    value = 0
    if isInt(address):
        value = int(address)
    elif address in address_table:
        value = address_table[address]
    else:
        value = next_address
        address_table[address] = next_address
        next_address += 1
    return f"{value:016b}"

def isInt(string):
    try:
        number = int(string)
        return True
    except Exception as e:
        return False

def c_instruction(code):
    if "=" in code and ";" in code:
        code = code.replace("=", ";")
        code = code.split(";")
    elif "=" in code:
        code = code.split("=")
        code.append("null")
    elif ";" in code:
        code = code.split(";")
        code.insert(0, "null")
    
    binary_code.append("111" + comp_bin(code[1]) + dest_bin(code[0]) + jump_bin(code[2]))


def dest_bin(dest):
    value = 0
    if "A" in dest:
        value += 4
    if "D" in dest:
        value += 2
    if "M" in dest:
        value += 1
    return (f'{value:03b}')

def comp_bin(comp):
    return (f"{comp_table[comp]:07b}")

def jump_bin(jump):
    return (f'{jump_table[jump]:03b}')

def main(file):
    hackToAsm(file)

if __name__ == "__main__":
    main("..\\project_files\\6\\pong\PongL")