import re

# Global variables shared
macros = {}
definitions = {}
memory = {}
routines = {}

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

def store_place(lines):
    global memory
    for line in lines:
        if line.startswith('.PLACE'):
            parts = line.split()
            addr = parts[1]
            data = parts[2]

            memory[addr] = data
    # Remove all .PLACE lines
    lines = [line for line in lines if not line.startswith('.PLACE')]
    return lines

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
