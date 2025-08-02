commands = {'push': 'C_PUSH', 'pop': 'C_POP', 'call': 'C_CALL', 'function': 'C_FUNCTION'}
flow = {'label': 'C_LABEL', 'goto': 'C_GOTO', 'if-goto': 'C_IF',
                 'return': 'C_RETURN'}
arithmetic = ['add', 'sub', 'neg', 'not', 'or', 'neg', 'lt', 'gt', 'eq']

parsed_code = []

current_class = ''

def parser(file):
    reader = open(file, 'r')
    for line in reader:
        line = line.strip() #Removes white space
        line = line.split(' ') #Splits line into array for easier management/lots of extra giblets though
        if line[0] == ' ' or line[0] == '\n' or line[0] == '//': #Removes any lines that aren't command lines
            continue
        parseCommands(line)
    return parsed_code

#Adds explict command to each line, making it easier to translate later
def parseCommands(line):
    global current_class
    if line[0] in commands:
        if line[1] == 'static': #Static variables are renamed to fit with their class
            line[1] = current_class + '_var'
        if line[0] == 'function': #Gets the current class name to rename static varibales
            function = line[1].split('.') #Splits the class from it's function
            current_class = function[0]
        parsed_code.append(f'{commands[line[0]]} {line[1]} {line[2]}') #Turns array back into string. Anything extra in the line is inherently removed
    elif line[0] in flow:
        if line[0] == 'return': #Single command, needs it's own check
            parsed_code.append(flow[line[0]])
        else:
            parsed_code.append(f'{flow[line[0]]} {line[1]}') #Turns array back into string. Anything extra in the line is inherently removed
    elif line[0] in arithmetic:
        parsed_code.append("C_ARITHMETIC " + line[0]) #Turns array back into string. Anything extra in the line is inherently removed
