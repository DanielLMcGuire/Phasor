# Phasor Standard Library 2.0.0 Documentation

## Table of Contents
1. [Introduction](#introduction)
2. [Standard I/O (`include_stdio`)](#standard-io)
3. [Mathematics (`include_stdmath`)](#mathematics)
4. [String Operations (`include_stdstr`)](#string-operations)
5. [Type Conversion (`include_stdtype`)](#type-conversion)
6. [File System (`include_stdfile`)](#file-system)
7. [System Functions (`include_stdsys`)](#system-functions)
8. [Regular Expressions (`include_stdregex`)](#regular-expressions)

---

## Introduction

The Phasor Standard Library (STDLIB) version 2.0.0 provides a comprehensive set of functions for common programming tasks. Each module must be explicitly included before its functions can be used. This document describes all available functions, their parameters, return values, and usage.

---

## Standard I/O

**Module Import:** `include_stdio()`

The Standard I/O module provides functions for input/output operations, including formatted printing and user input.

### Functions

#### `c_fmt(format, ...args)`
Formats a string using C-style format specifiers.

**Parameters:**
- `format` (string): Format string containing specifiers
- `...args` (variadic): Values to format

**Format Specifiers:**
- `%s` - String
- `%d` - Integer (decimal)
- `%f` - Float
- `%%` - Literal percent sign

**Returns:** Formatted string

**Example:**
```javascript
c_fmt("Hello %s, you are %d years old", "Alice", 25)
// Returns: "Hello Alice, you are 25 years old"
```

#### `prints(text)`
Prints text to standard output without a newline.

**Parameters:**
- `text` (string): Text to print

**Returns:** Empty string

#### `printf(format, ...args)`
Prints formatted text to standard output without a newline.

**Parameters:**
- `format` (string): Format string
- `...args` (variadic): Values to format

**Returns:** Empty string

#### `puts(text)`
Prints text to standard output with a newline. Processes escape sequences.

**Parameters:**
- `text` (string): Text to print

**Returns:** Empty string

**Escape Sequences:**
- `\n` - Newline
- `\t` - Tab
- `\r` - Carriage return
- `\\` - Backslash
- `\"` - Double quote
- `\'` - Single quote
- `\0` - Null character

#### `putf(format, ...args)`
Prints formatted text to standard output with a newline.

**Parameters:**
- `format` (string): Format string
- `...args` (variadic): Values to format

**Returns:** Empty string

#### `gets()`
Reads a line of input from standard input.

**Parameters:** None

**Returns:** String containing user input (with escape sequences processed)

#### `puts_error(text)`
Prints text to standard error output with a newline.

**Parameters:**
- `text` (string): Text to print

**Returns:** Result of error output operation

#### `putf_error(format, ...args)`
Prints formatted text to standard error output with a newline.

**Parameters:**
- `format` (string): Format string
- `...args` (variadic): Values to format

**Returns:** Result of error output operation

#### `msgbox(title, message)`
Displays a message box (platform-dependent).

**Parameters:**
- `title` (string): Message box title
- `message` (string): Message to display

**Returns:** Platform-specific result code (Windows: message box result)

**Note:** On non-Windows platforms, this prints to console instead.

---

## Mathematics

**Module Import:** `include_stdmath()`

The Mathematics module provides common mathematical operations and functions.

### Functions

#### `math_sqrt(x)`
Calculates the square root of a number.

**Parameters:**
- `x` (number): Value to calculate square root of

**Returns:** Square root of x

#### `math_pow(base, exponent)`
Raises a number to a power.

**Parameters:**
- `base` (number): Base value
- `exponent` (number): Exponent value

**Returns:** base^exponent

#### `math_abs(x)`
Returns the absolute value of a number.

**Parameters:**
- `x` (number): Input value

**Returns:** Absolute value of x

#### `math_floor(x)`
Rounds down to the nearest integer.

**Parameters:**
- `x` (number): Value to floor

**Returns:** Largest integer less than or equal to x

#### `math_ceil(x)`
Rounds up to the nearest integer.

**Parameters:**
- `x` (number): Value to ceil

**Returns:** Smallest integer greater than or equal to x

#### `math_round(x)`
Rounds to the nearest integer.

**Parameters:**
- `x` (number): Value to round

**Returns:** Nearest integer to x

#### `math_min(a, b)`
Returns the smaller of two values.

**Parameters:**
- `a` (number): First value
- `b` (number): Second value

**Returns:** Minimum of a and b

#### `math_max(a, b)`
Returns the larger of two values.

**Parameters:**
- `a` (number): First value
- `b` (number): Second value

**Returns:** Maximum of a and b

#### `math_log(x)`
Calculates the natural logarithm (base e).

**Parameters:**
- `x` (number): Input value

**Returns:** ln(x)

#### `math_exp(x)`
Calculates e raised to the power x.

**Parameters:**
- `x` (number): Exponent

**Returns:** e^x

#### `math_sin(x)`
Calculates the sine of an angle.

**Parameters:**
- `x` (number): Angle in radians

**Returns:** sin(x)

#### `math_cos(x)`
Calculates the cosine of an angle.

**Parameters:**
- `x` (number): Angle in radians

**Returns:** cos(x)

#### `math_tan(x)`
Calculates the tangent of an angle.

**Parameters:**
- `x` (number): Angle in radians

**Returns:** tan(x)

---

## String Operations

**Module Import:** `include_stdstr()`

The String Operations module provides functions for string manipulation and a StringBuilder for efficient string concatenation.

### String Functions

#### `len(str)`
Returns the length of a string.

**Parameters:**
- `str` (string): Input string

**Returns:** Integer length of the string

#### `char_at(str, index)`
Returns the character at a specific index.

**Parameters:**
- `str` (string): Input string
- `index` (integer): Zero-based character index

**Returns:** Single character string at the specified index, or empty string if out of bounds

#### `substr(str, start, [length])`
Extracts a substring from a string.

**Parameters:**
- `str` (string): Input string
- `start` (integer): Starting index (zero-based)
- `length` (integer, optional): Number of characters to extract. If omitted, extracts to end of string

**Returns:** Substring, or empty string if start is out of bounds

#### `concat(str1, str2, ...)`
Concatenates multiple strings together.

**Parameters:**
- `str1, str2, ...` (strings): Strings to concatenate

**Returns:** Concatenated string

#### `to_upper(str)`
Converts a string to uppercase.

**Parameters:**
- `str` (string): Input string

**Returns:** Uppercase version of the string

#### `to_lower(str)`
Converts a string to lowercase.

**Parameters:**
- `str` (string): Input string

**Returns:** Lowercase version of the string

#### `starts_with(str, prefix)`
Checks if a string starts with a specific prefix.

**Parameters:**
- `str` (string): String to check
- `prefix` (string): Prefix to look for

**Returns:** Boolean - true if string starts with prefix, false otherwise

#### `ends_with(str, suffix)`
Checks if a string ends with a specific suffix.

**Parameters:**
- `str` (string): String to check
- `suffix` (string): Suffix to look for

**Returns:** Boolean - true if string ends with suffix, false otherwise

### StringBuilder Functions

The StringBuilder provides efficient string concatenation for building large strings.

#### `sb_new()`
Creates a new StringBuilder instance.

**Parameters:** None

**Returns:** StringBuilder handle (integer)

#### `sb_append(handle, text)`
Appends text to a StringBuilder.

**Parameters:**
- `handle` (integer): StringBuilder handle
- `text` (string): Text to append

**Returns:** The same handle for chaining operations

**Example:**
```javascript
var sb = sb_new()
sb_append(sb, "Hello ")
sb_append(sb, "World")
var result = sb_to_string(sb)
```

#### `sb_to_string(handle)`
Converts a StringBuilder to a string.

**Parameters:**
- `handle` (integer): StringBuilder handle

**Returns:** Concatenated string

#### `sb_free(handle)`
Frees a StringBuilder instance.

**Parameters:**
- `handle` (integer): StringBuilder handle

**Returns:** Length of the string that was in the StringBuilder

---

## Type Conversion

**Module Import:** `include_stdtype()`

The Type Conversion module provides functions to convert between different data types.

### Functions

#### `to_int(value)`
Converts a value to an integer.

**Parameters:**
- `value` (any): Value to convert

**Conversion Rules:**
- Integer: Returns as-is
- Float: Truncates to integer
- String: Parses as integer (returns 0 on failure)
- Boolean: Returns 1 for true, 0 for false
- Other: Returns 0

**Returns:** Integer value

#### `to_float(value)`
Converts a value to a floating-point number.

**Parameters:**
- `value` (any): Value to convert

**Returns:** Float value

#### `to_string(value)`
Converts a value to a string representation.

**Parameters:**
- `value` (any): Value to convert

**Returns:** String representation of the value

#### `to_bool(value)`
Converts a value to a boolean.

**Parameters:**
- `value` (any): Value to convert

**Conversion Rules:**
- Boolean: Returns as-is
- Integer: Returns false for 0, true otherwise
- String: Returns false for empty string, true otherwise
- Other: Returns false

**Returns:** Boolean value

---

## File System

**Module Import:** `include_stdfile()`

The File System module provides comprehensive file and directory operations.

### File I/O Functions

#### `fread(path)`
Reads the entire contents of a file.

**Parameters:**
- `path` (string): Path to the file

**Returns:** File contents as string, or null if file cannot be opened

#### `fwrite(path, content)`
Writes content to a file, overwriting existing content.

**Parameters:**
- `path` (string): Path to the file
- `content` (string): Content to write

**Returns:** Boolean true on success

**Throws:** Error if file cannot be opened for writing

#### `fappend(path, content)`
Appends content to the end of a file.

**Parameters:**
- `path` (string): Path to the file
- `content` (string): Content to append

**Returns:** Boolean true on success

**Throws:** Error if file cannot be opened for writing

#### `freadln(path, lineNumber)`
Reads a specific line from a file.

**Parameters:**
- `path` (string): Path to the file
- `lineNumber` (integer): Zero-based line number

**Returns:** Content of the specified line

**Throws:** Error if file cannot be opened

#### `fwriteln(path, lineNumber, content)`
Writes content to a specific line in a file.

**Parameters:**
- `path` (string): Path to the file
- `lineNumber` (integer): Zero-based line number
- `content` (string): Content to write

**Behavior:**
- Reads entire file, updates the specified line, writes back
- If line number exceeds file length, empty lines are added

**Returns:** Boolean true on success

**Throws:** Error if file cannot be opened

### File Operations

#### `fexists(path)`
Checks if a file or directory exists.

**Parameters:**
- `path` (string): Path to check

**Returns:** Boolean true if exists, false otherwise

#### `fmk(path)`
Creates a new empty file.

**Parameters:**
- `path` (string): Path to the file to create

**Returns:** Boolean true on success

**Throws:** Error if file cannot be created

#### `frm(path)`
Deletes a file.

**Parameters:**
- `path` (string): Path to the file

**Returns:** Boolean true if file was deleted, false if file doesn't exist

#### `frn(oldPath, newPath)`
Renames or moves a file.

**Parameters:**
- `oldPath` (string): Current path
- `newPath` (string): New path

**Returns:** Boolean true if successful, false if source doesn't exist

#### `fcp(source, destination, [overwrite])`
Copies a file.

**Parameters:**
- `source` (string): Source file path
- `destination` (string): Destination file path
- `overwrite` (boolean, optional): Whether to overwrite if destination exists

**Returns:** Boolean true on success, false on failure

**Errors Logged to stderr:**
- "Source file doesn't exist"
- "Failed to open source file"
- "Failed to open destination file"
- "Error during file copy"

#### `fmv(source, destination)`
Moves a file (copy then delete).

**Parameters:**
- `source` (string): Source file path
- `destination` (string): Destination file path

**Returns:** Null value

### Directory Operations

#### `fmkdir(path)`
Creates a new directory.

**Parameters:**
- `path` (string): Path for the new directory

**Returns:** Boolean true if created, false if already exists

#### `frmdir(path)`
Removes a directory.

**Parameters:**
- `path` (string): Directory path to remove

**Returns:** Boolean true

**Note:** Directory must be empty to be removed

#### `freaddir(path)`
Lists contents of a directory.

**Parameters:**
- `path` (string): Directory path

**Returns:** String with filenames separated by newlines, or error message on failure

#### `fcd([path])`
Gets or sets the current working directory.

**Parameters:**
- `path` (string, optional): New working directory path

**Returns:**
- If no arguments: Current working directory as string
- If path provided: New working directory as string, or false if path invalid

### File Properties and Statistics

#### `fpropset(path, property, value)`
Sets a file property (timestamps, etc.).

**Parameters:**
- `path` (string): File path
- `property` (char): Property identifier
- `value` (integer): New value (typically epoch time)

**Returns:** Result of property set operation

**Note:** Value must be non-negative

#### `fpropget(path, property)`
Gets a file property.

**Parameters:**
- `path` (string): File path
- `property` (char): Property identifier

**Returns:** Property value

#### `fstat(path)`
Gets detailed file statistics.

**Parameters:**
- `path` (string): File path

**Returns:** FileStat struct with fields:
- `mode` (integer): File type and permissions (mode_t format)
  - File type bits: 0x4000 (directory), 0x8000 (regular file), 0xA000 (symlink)
  - Permission bits: owner (rwx), group (rwx), others (rwx)
- `nlink` (integer): Number of hard links
- `uid` (integer): User ID of owner
- `gid` (integer): Group ID of owner
- `size` (integer): File size in bytes

**Returns:** Null on error (error logged to stderr)

---

## System Functions

**Module Import:** `include_stdsys()`

The System Functions module provides access to system-level operations, time functions, and runtime control.

### Time Functions

#### `time()`
Gets the current time in milliseconds since epoch.

**Parameters:** None

**Returns:** Integer milliseconds since Unix epoch

#### `timef(format)`
Gets formatted current time.

**Parameters:**
- `format` (string): strftime-compatible format string

**Common Format Codes:**
- `%Y` - Year (4 digits)
- `%m` - Month (01-12)
- `%d` - Day (01-31)
- `%H` - Hour (00-23)
- `%M` - Minute (00-59)
- `%S` - Second (00-59)
- `%A` - Weekday name
- `%B` - Month name

**Returns:** Formatted time string, or single space on error

#### `sleep(milliseconds)`
Pauses execution for a specified duration.

**Parameters:**
- `milliseconds` (integer): Duration to sleep

**Returns:** Single space string

### System Information

#### `sys_os()`
Gets the operating system name.

**Parameters:** None

**Returns:** String identifying the OS:
- `"NT"` - Windows
- `"Linux"` - Linux
- `"Darwin"` - macOS
- `"FreeBSD"` - FreeBSD
- `"Unknown"` - Other

#### `sys_env(name)`
Gets an environment variable value.

**Parameters:**
- `name` (string): Environment variable name

**Returns:** Environment variable value, or empty string if not found

#### `sys_argc()`
Gets the number of command-line arguments.

**Parameters:** None

**Returns:** Integer count of arguments

#### `sys_argv([index])`
Gets command-line arguments.

**Parameters:**
- `index` (integer, optional): Argument index

**Returns:**
- If no index: Struct with fields `arg0`, `arg1`, etc., containing all arguments
- If index provided: String value of the argument at that index

### Console Functions

#### `clear()`
Clears the console screen using ANSI escape codes.

**Parameters:** None

**Returns:** Empty string

#### `wait_for_input()`
Waits for user to press Enter.

**Parameters:** None

**Returns:** Empty string

### Execution Control

#### `sys_execute(command)`
Executes a shell command.

**Parameters:**
- `command` (string): Shell command to execute

**Returns:** Command execution result

#### `error(message)`
Throws a runtime error with a message.

**Parameters:**
- `message` (string): Error message

**Throws:** Runtime error with the specified message

**Side Effect:** Resets the VM after throwing

#### `reset()`
Resets the virtual machine state.

**Parameters:** None

**Returns:** Null value

**Warning:** Can cause crashes if not used properly

#### `shutdown(exitCode)`
Terminates the program with an exit code.

**Parameters:**
- `exitCode` (integer): Exit code to return to the operating system

**Returns:** Does not return (program terminates)

**Side Effect:** Resets VM and exits the program

---

## Regular Expressions

**Module Import:** `include_stdregex()`

The Regular Expressions module provides pattern matching and text processing capabilities using regex.

### Functions

#### `regex_match(pattern, text)`
Tests if a pattern matches an entire string.

**Parameters:**
- `pattern` (string): Regular expression pattern
- `text` (string): Text to match against

**Returns:** Boolean true if entire text matches pattern, false otherwise

**Throws:** Error on invalid regex pattern

**Example:**
```javascript
regex_match("^[0-9]+$", "12345")  // true
regex_match("^[0-9]+$", "123abc") // false
```

#### `regex_search(pattern, text, [start], [end])`
Searches for a pattern within a string.

**Parameters:**
- `pattern` (string): Regular expression pattern
- `text` (string): Text to search
- `start` (integer, optional): Start position (default: 0)
- `end` (integer, optional): End position (default: length of text)

**Returns:** First match as string, or empty string if not found

**Throws:** Error on invalid regex pattern or invalid start/end positions

**Example:**
```
regex_search("[0-9]+", "abc123def") // "123"
```

#### `regex_findall(pattern, text)`
Finds all occurrences of a pattern in text.

**Parameters:**
- `pattern` (string): Regular expression pattern
- `text` (string): Text to search

**Returns:** RegexMatches struct with:
- `count` (integer): Number of matches found
- `0`, `1`, `2`, ... (RegexMatch structs): Individual matches

Each RegexMatch struct contains:
- `text` (string): Matched text
- `position` (integer): Position in original string

**Throws:** Error on invalid regex pattern

**Example:**
```javascript
var matches = regex_findall("[0-9]+", "a1b23c456")
// matches.count = 3
// matches.0.text = "1"
// matches.1.text = "23"
// matches.2.text = "456"
```

#### `regex_split(pattern, text, [maxSplit])`
Splits text by pattern matches.

**Parameters:**
- `pattern` (string): Regular expression pattern to split on
- `text` (string): Text to split
- `maxSplit` (integer, optional): Maximum number of splits (-1 for unlimited)

**Returns:** SplitResult struct with:
- `count` (integer): Number of parts
- `0`, `1`, `2`, ... (strings): Split parts

**Throws:** Error on invalid regex pattern

**Example:**
```javascript
var parts = regex_split("[,;]", "a,b;c,d")
// parts.count = 4
// parts.0 = "a"
// parts.1 = "b"
// parts.2 = "c"
// parts.3 = "d"
```

#### `regex_replace(pattern, text, replacement, [flags])`
Replaces pattern matches with replacement text.

**Parameters:**
- `pattern` (string): Regular expression pattern
- `text` (string): Text to search
- `replacement` (string): Replacement text
- `flags` (string, optional): Regex flags (see below)

**Regex Flags:**
- `e` - ECMAScript syntax (default)
- `a` - AWK syntax
- `g` - Grep syntax
- `p` - Egrep syntax
- `i` - Case insensitive
- `m` - Multiline mode
- `n` - No sub-expressions
- `o` - Optimize regex
- `c` - Locale-specific collation
- `f` - sed-style formatting
- `r` - Format with no copy
- `d` - Replace first only

**Returns:** String with replacements made

**Throws:** Error on invalid regex pattern

**Example:**
```javascript
regex_replace("[0-9]+", "a1b2c3", "X")     // "aXbXcX"
regex_replace("[0-9]+", "a1b2c3", "X", "d") // "aXb2c3" (first only)
```

---

## Error Handling

Most functions that can fail will either:
1. **Return a special value** (null, false, empty string) to indicate failure
2. **Throw an error** that should be caught by the calling code

Functions that throw errors typically do so for:
- Invalid arguments (wrong type, out of range)
- File I/O failures
- Invalid regex patterns
- System operation failures

---

## Best Practices

1. **Check return values:** Many functions return false or null on failure
2. **Use StringBuilder for large string concatenation:** More efficient than repeated concatenation
3. **Close resources:** While not explicitly required, avoid keeping unnecessary file handles
4. **Validate user input:** Use type conversion functions with error checking
5. **Handle regex errors:** Regex functions throw on invalid patterns
6. **Use appropriate I/O functions:** `puts` for simple output, `putf` for formatted output
7. **Check file existence:** Use `fexists` before file operations to avoid errors

---

## Version History

### 1.0.0
- Private, Internal Release

### 2.0.0
- Redesign to fit hybrid architecture with new registers
- Complete standard library with I/O, math, strings, files, system, and regex support
- StringBuilder for efficient string operations
- Comprehensive file system operations including directories
- File statistics and property management
- Full regex support with match, search, findall, split, and replace
- Cross-platform system functions

---

## License and Copyright

Phasor Language Standard Library specification 2.0.0.

(C) 2025 Daniel McGuire

Use, redistribution, or modification of The Phasor Standard Library is granted under the terms and conditions of the Phasor Open License 1.0 and potentially the Phasor Binary EULA.
