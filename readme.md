# minibash-shell

## Link to video and demo of code: 
https://drive.google.com/file/d/19s1QCUotJxLb93VCmH1lbYTs7qu0oQQo/view?usp=sharing

## Overview
This repository contains a C program named `minibash` which acts as a simple shell that goes into an infinite loop waiting for user's commands. Once a command is entered, the program assembles and executes each command using `fork()`, `exec()`, and other system calls as required. 

## Features
- **Execute Commands:** Execute commands entered by the user using `fork()` and `exec()`.
- **Special Characters:** Handle special characters like `#`, `~`, `+`, `|`, `<`, `>`, `>>`, `;`, `&&`, and `||`.
- **Process Management:** Run processes in the background and bring them to the foreground.
- **File Operations:** Perform file operations like word count, file concatenation, and redirection.

## Command Line Arguments
- `minibash$dter`: Kills the current minibash terminal.
- `#`: Counts the number of words present in a specified `.txt` file.
  - Example: `minibash$ # sample.txt` outputs the number of words in `sample.txt`.
- `~`: Concatenates specified `.txt` files.
  - Example: `minibash$ sample1.txt ~ sample2.txt ~ temp1.txt ~ temp2.txt` concatenates the files in order.
- `+`: Runs a process in the background.
  - Example: `minibash$ ex10 30 20 a +` runs `ex10 30 20 a` in the background.
  - `minibash$ fore` brings the last background process to the foreground.
- `|`: Piping operations (up to 4 pipes supported).
  - Example: `minibash$ ls -l -t -a | grep *.txt | wc | wc -w`.
- `<, >, >>`: Input and output redirection.
  - Example: `minibash$ grep to < sample.txt` for input redirection.
  - Example: `minibash$ ls -1 > dirlist.txt` for output redirection.
  - Example: `minibash$ ls -1 >> dirlist.txt` for output redirection with append.
- `;`: Sequential execution of commands (up to 4 commands).
  - Example: `minibash$ date ; pwd ; ls -l -t -a`.
- `&&, ||`: Conditional execution (up to 4 operators supported, combination of && and || possible).
  - Example: `minibash$ ex1 && ex2 && ex3 && ex4`.
  - Example: `minibash$ c1 && c2 || c3 && c4`.

## Usage Examples
- Kill minibash terminal: `$ minibash$dter`
- Word count in a file: `$ minibash$ # sample.txt`
- Concatenate files: `$ minibash$ sample1.txt ~ sample2.txt`
- Run a process in the background: `$ minibash$ ex10 30 20 a +`
- Piping commands: `$ minibash$ ls -l -t -a | grep *.txt | wc | wc -w`
- Input redirection: `$ minibash$ grep to < sample.txt`
- Output redirection: `$ minibash$ ls -1 > dirlist.txt`
- Sequential execution: `$ minibash$ date ; pwd ; ls -l -t -a`
- Conditional execution: `$ minibash$ ex1 && ex2 && ex3 && ex4`

## Mandatory Requirements
- Avoid creating a "fork bomb". Use `$killall -u username` periodically.

## Author
Meet Patel
