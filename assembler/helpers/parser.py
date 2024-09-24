import re

def remove_comments(lines):
    return [re.sub(r';.*', '', line) for line in lines]

def remove_empty_lines(lines):
    return [line for line in lines if line.strip()]
