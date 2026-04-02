import re
import os

def resolve_imports(lines, base_dir, _seen=None):
    """Recursively replace @import "file" lines with the file's contents."""
    if _seen is None:
        _seen = set()
    result = []
    for line in lines:
        stripped = line.strip()
        if stripped.startswith('@import'):
            # Support: @import "path" or @import path
            match = re.match(r'@import\s+"([^"]+)"', stripped) or \
                    re.match(r'@import\s+(\S+)', stripped)
            if not match:
                print(f"Error: malformed @import: {stripped}")
                continue
            rel_path = match.group(1)
            abs_path = os.path.join(base_dir, rel_path)
            if abs_path in _seen:
                continue  # guard against circular imports
            _seen.add(abs_path)
            try:
                with open(abs_path, 'r') as f:
                    imported = f.readlines()
                result.extend(resolve_imports(imported, os.path.dirname(abs_path), _seen))
            except FileNotFoundError:
                print(f"Error: @import file not found: {abs_path}")
        else:
            result.append(line)
    return result

def remove_comments(lines):
    return [re.sub(r';.*', '', line) for line in lines]

def remove_empty_lines(lines):
    return [line for line in lines if line.strip()]
