import os
from Parser import parser
from Code_writer import code_writer
parsed_program = []

def main():
    #These three variables are split merely to make changing program and files easier
    projects = '..\\..\\project_files\\'
    project = '8\\'
    program = 'NestedCall'
    #program = input() #If you wnat to manually enter the program folder... can't change any other directory though
    directory = projects + project + program #Creates the single string directory path to the folder with program in it

    files = get_files(directory) #Get all .vm files in specified folder
    parse_files(files) #Pass all files to helper function to parse the program
    code_writer(parsed_program, directory, program) #Translates parsed program into .asm and further into hack. Saves file .asm and .hack in same folder as the .vm files


def get_files(directory):
    files = []
    for file in os.listdir(directory):
        if file[-3:] == '.vm': #Looks only for files that end in .vm
            files.append(f'{directory}\\{file}') #Adds file to files array with full string path
    return files

def parse_files(files):
    for file in files:
        parse = parser(file) #Feeds each file to be parsed and combined into one file
    for line in parse:
        parsed_program.append(line) #Adds all lines to Main.py 'parsed_program' array. Seperate for loop is used because I haven't fixed a bug I created yet.

main()