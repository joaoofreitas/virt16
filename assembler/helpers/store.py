import re

# Global variables shared
macros = {}
definitions = {}
memory = {}
routines = {}
places = {}

def store_macros(lines):
    global macros
    for line in lines:
        if line.startswith('@macro'):
            parts = line.split()
            name = parts[1]
            # args go inside [ ] and are separated by commas
            args = parts[2].strip('[]').split(',')
            # remove args empty strings
            args = [arg for arg in args if arg]
            # body is everything after the first line until @endmacro
            body = []
            for line in lines[lines.index(line)+1:]:
                if line.startswith('@endmacro'):
                    break
                body.append(line)
                # Remove line from lines
                lines.remove(line)

            # Remove whitespace and \n from the args and body
            args = [arg.strip() for arg in args]
            body = [line.strip() for line in body]
            macros[name] = (args, body)
    
    # Remove all @macro and @endmacro lines
    lines = [line for line in lines if not line.startswith('@macro') and not line.startswith('@endmacro')]

    return lines



def store_definitions(lines):
    global definitions
    for line in lines:
        if line.startswith('@define'):
            parts = line.split()
            name = parts[1]
            value = parts[2]
            definitions[name] = value
    # Remove all @define lines
    lines = [line for line in lines if not line.startswith('@define')]
    return lines

# .PLACE is used to store ASCII text or Array of numbers in hex.
# .PLACE 0x2000 "Hello, World!"
# .PLACE 0x2000 [0x1048, 0x1045 ...]
# The arrays can be multi-line

def store_place(lines):
    global places

    # To store the cleaned lines
    cleaned_lines = []
    i = 0
    
    while i < len(lines):
        line = lines[i].strip()
        
        if line.startswith('.PLACE'):
            parts = line.split()
            address = parts[1]  # Get the address
            
            # If it's a string (enclosed in double quotes)
            if parts[2].startswith('"'):
                text = line.split('"')[1]  # Extract text between quotes
                places[address] = text
                i += 1  # Move to the next line
            # If it's an array (starts with a '[')
            elif parts[2] == '[':
                array = []
                i += 1  # Move to the next line (start of array content)
                
                while i < len(lines):
                    array_line = lines[i].strip()
                    
                    # If we find the closing ']', stop processing the array
                    if array_line.endswith(']'):
                        array.extend(array_line[:-1].split(','))  # Add the last part before ']'
                        break
                    
                    # Otherwise, process the current line, removing any trailing commas
                    array.extend(array_line.split(','))
                    i += 1
                
                # Clean up array by stripping extra whitespace
                array = [value.strip() for value in array if value.strip()]
                places[address] = array
                i += 1  # Move past the closing ']'
            else:
                print(f"Error: Unrecognized format for .PLACE at line {i + 1}")
                i += 1
        else:
            # Keep non-.PLACE lines
            cleaned_lines.append(lines[i])
            i += 1

    return cleaned_lines
    

def substitute_macros_and_defs(lines):
    # if line is a macro (starts with @), replace it with the body
    for i, line in enumerate(lines):
        # line starts with @
        matches = re.findall(r'@\w+', line)
        for match in matches:
            name = match[1:]
            if name in macros:
                # Check if the macro is defined
                args, body = macros[name]
                b_copy = body.copy()
                l = line.strip().split()
                if (len(l) - 1) != len(args):
                    print(f"Error: Incorrect number of arguments for macro {name}")
                    break

                # Replace args with values
                for j, arg in enumerate(args):
                    for k, word in enumerate(b_copy):
                        b_copy[k] = re.sub(arg, l[j+1], word)

                # Insert body into lines
                lines[i] = '    ' + b_copy[0] + '\n'
                for line in b_copy[1:]:
                    i += 1
                    lines.insert(i, '    ' + line + '\n')

            else:
                print(f"Error: Macro {match} not defined")
        


    # Definitions start with %NAME and are replaced with the value
    for i, line in enumerate(lines):
        matches = re.findall(r'%\w+', line)
        for match in matches:
            name = match[1:]
            if name in definitions:
                lines[i] = re.sub(match, definitions[name], lines[i])
            else:
                print(f"Error: Definition {match} not defined")
    return lines

def encapsulate_routine(lines):
    global routines
    # Routines start with .NAME: (so starts with a dot ends with a colon)
    for i, line in enumerate(lines):
        if line.startswith('.'):
            name = line.strip().strip(':')
            body = []
            for line in lines[i+1:]:
                if line.startswith('.'):
                    break
                body.append(line.strip())
            routines[name] = body
