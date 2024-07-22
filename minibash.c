#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/signal.h>
#include<unistd.h> //fork
#include<sys/wait.h>
#include<ctype.h> // ‘isspace’
#include <sys/types.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#define MAX_INPUT 1024
#define MAX_ARGS 4

#define MAX_BG_PROCESSES 10

typedef struct {
    pid_t pid;
    char command[256];
} BackgroundProcess;

BackgroundProcess bg_processes[MAX_BG_PROCESSES];
int bg_count = 0;

int is_valid_path(const char *path) {
    if (access(path, F_OK) != -1) {
        // Path exists
        return 1;
    } else {
        // Path does not exist
        return 0;
    }
}
void trim_trailing_whitespace(char *str) {
    if (str == NULL)
    {
        return; // Handle null pointer gracefully
    }

    // Find the first non-whitespace character
    int start = 0;
    while (isspace(str[start]))
    {
        start++;
    }

    // Find the last non-whitespace character
    int end = strlen(str) - 1;
    while (end >= start && isspace(str[end]))
    {
        end--;
    }

    // Shift the characters and null-terminate the string
    if (end >= start)
    {
        int i = 0;
        for (int j = start; j <= end; j++)
        {
            str[i++] = str[j];
        }
        str[i] = '\0';
    }
    else
    {
        // The string is entirely whitespace, set it to empty string
        str[0] = '\0';
    }
}
void execute_command(char *args[]) {
    int pid = fork();
    if (pid == 0) {
        sleep(1); // try to avoiding zombie condition.
        printf("In: Execute Command: . %s .\n",args[2]);
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
}
int check_characters_in_string(const char *str1, const char *str2) {
    const char *ptr1 = str1;
    const char *ptr2 = str2;

    while (*ptr1) {
        if (*ptr1 == *ptr2) {
            // Found a match for the first character of str2
            const char *temp1 = ptr1;
            const char *temp2 = ptr2;

            // Check if the rest of str2 matches
            while (*temp2 && *temp1 == *temp2) {
                temp1++;
                temp2++;
            }

            // If we've reached the end of str2, we found a full match
            if (*temp2 == '\0') {
                return 1;
            }
        }
        ptr1++;
    }
    return 0; // No matching subsequence found
}
char *custom_strtok(char *str, const char *delimiters)
{
    static char *savedStr;
    if (str){
        savedStr = str;
    }
    else if (!savedStr){
        return NULL;
    }

    // Skip leading delimiters
    while (*savedStr && strchr(delimiters, *savedStr)){
        savedStr++;
    }
    if (!*savedStr){
        return NULL;
    }
    char *token = savedStr;

    // Find the end of the token
    while (*savedStr && !strchr(delimiters, *savedStr)){
        savedStr++;
    }

    if (*savedStr){
        *savedStr = '\0';
        savedStr++;
    }
    else{
        savedStr = NULL;
    }
    return token;
}

// Function to split input string based on provided delimiters with a maximum of 3 separators
void split_input_with_delimiters(char *input, int *argc, char ***argv, const char *delimiters)
{
    // Default delimiter to space if none provided
    if (delimiters == NULL || strlen(delimiters) == 0){
        delimiters = " ";
    }

    // Initialize argc
    *argc = 0;

    // Copy the input string to avoid modifying the original
    char *input_copy = strdup(input);

    // Allocate memory for argv (maximum 4 elements)
    *argv = malloc(4 * sizeof(char *));

    // Tokenize the input string based on the provided delimiters
    char *token = custom_strtok(input_copy, delimiters);
    while (token != NULL){
        (*argv)[(*argc)++] = strdup(token);
        token = custom_strtok(NULL, delimiters);
    }
    // Free the copied input string
    free(input_copy);
}
// Function to handle execution of command sequentially.
void handle_sequential(int count_command,char **value_command){
    for (int i = 0; i < count_command; i++) {
        char **argv;
        int argc;
        split_input_with_delimiters(value_command[i],&argc,&argv," ");
        execute_command(argv);
    }
}
void handle_concetination(int count_command,char **value_command){
    char **value_command_2 = malloc((sizeof(char*)*(count_command+1))+1);
    value_command_2[0] = malloc(sizeof(char)*4);
    strcpy(value_command_2[0],"cat");
    for(int i=0;i<count_command;i++){
        value_command_2[i+1]=malloc(sizeof(char)*strlen(value_command[i]));
        trim_trailing_whitespace(value_command[i]);
        strcpy(value_command_2[i+1],value_command[i]);
    }
    execute_command(value_command_2); 
}    
void handle_pipe(int count_command, char **value_command) {
    int pipefd[2];
    pid_t pid;
    int in_fd = 0;

    for (int i = 0; i < count_command; i++) {
        pipe(pipefd);

        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child process
            if (in_fd != 0) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (i < count_command - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
            }

            close(pipefd[0]);
            close(pipefd[1]);

            char **argv;
            int argc;
            split_input_with_delimiters(value_command[i], &argc, &argv, " ");

            // Ensure the last element of argv is NULL
            argv = realloc(argv, (argc + 1) * sizeof(char *));
            argv[argc] = NULL;

            execvp(argv[0], argv);
            perror("execvp");
            exit(1);
        } else {
            // Parent process
            if (in_fd != 0) {
                close(in_fd);
            }
            close(pipefd[1]);
            in_fd = pipefd[0];
        }
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);
}
void handle_counting(int count_command, char **value_command){
    char **value_command_2 = malloc((sizeof(char*)*(count_command+1))+1);
    value_command_2[0] = malloc(sizeof(char)*2);
    strcpy(value_command_2[0],"wc");
    value_command_2[1] = malloc(sizeof(char)*2);
    strcpy(value_command_2[1],"-w");
    for(int i=1;i<count_command;i++){
        value_command_2[i+1]=malloc(sizeof(char)*strlen(value_command[i]));
        trim_trailing_whitespace(value_command[i]);
        strcpy(value_command_2[i+1],value_command[i]);
    }
    execute_command(value_command_2); 
} 
void handle_background(int count_command, char **value_command) {
    if (count_command < 1 || count_command > 4) {
        printf("Invalid number of arguments. Must be between 1 and 4.\n");
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        setpgid(0, 0);
        
        char **args = malloc((count_command + 1) * sizeof(char *));
        for (int i = 0; i < count_command; i++) {
            args[i] = strdup(value_command[i]);
        }
        args[count_command] = NULL;

        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        printf("[%d] %d\n", bg_count + 1, pid);
        if (bg_count < MAX_BG_PROCESSES) {
            bg_processes[bg_count].pid = pid;
            snprintf(bg_processes[bg_count].command, sizeof(bg_processes[bg_count].command), "%s", value_command[0]);
            bg_count++;
        } else {
            printf("Maximum number of background processes reached.\n");
        }
    } else {
        perror("fork");
    }
}

void handle_foreground() {
    if (bg_count <= 0) {
        printf("No background processes.\n");
        return;
    }

    bg_count--;
    pid_t pid = bg_processes[bg_count].pid;
    printf("Bringing process %d (%s) to foreground\n", pid, bg_processes[bg_count].command);

    // Give terminal control to the process
    tcsetpgrp(STDIN_FILENO, pid);

    // Send SIGCONT in case it was stopped
    kill(-pid, SIGCONT);

    int status;
    waitpid(pid, &status, WUNTRACED);

    // Take back terminal control
    tcsetpgrp(STDIN_FILENO, getpgrp());

    if (WIFSTOPPED(status)) {
        printf("Process %d stopped\n", pid);
        // If the process was stopped, put it back in the background list
        bg_count++;
    }
}
void handle_redirect(int count_command, char **value_command, int redirect_type) {
    int pid;
    trim_trailing_whitespace(value_command[1]);
    char **argv;
    int argc;
    split_input_with_delimiters(value_command[0], &argc, &argv, " ");
    
    pid = fork();
    if (pid < 0) {
        perror("fork");
    } else if (pid == 0) {
        int fd;
        int flags;
        int target_fd;

        switch (redirect_type) {
            case 0: // Output redirect
                flags = O_WRONLY | O_CREAT | O_TRUNC;
                target_fd = STDOUT_FILENO;
                break;
            case 1: // Input redirect
                flags = O_RDONLY;
                target_fd = STDIN_FILENO;
                break;
            case 2: // Append redirect
                flags = O_WRONLY | O_CREAT | O_APPEND;
                target_fd = STDOUT_FILENO;
                break;
            default:
                fprintf(stderr, "Invalid redirect type\n");
                exit(1);
        }

        fd = open(value_command[1], flags, 0644);
        if (fd == -1) {
            perror("open");
            exit(1);
        }
        
        dup2(fd, target_fd);
        close(fd);
        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    } else {
        wait(NULL);
    }
}int main()
{
    char input[MAX_INPUT];
    int count_command, argc;
    char **value_command, **argv;
    // Set up signal handling
    signal(SIGTTOU, SIG_IGN);
    while (1 == 1)
    {
        printf("minibash$ ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }
        input[strcspn(input, "\n")] = 0;
        printf("Input is: %s\n", input);
        if (strcmp(input, "dter") == 0)
        {
            exit(0);
        }
        else if(check_characters_in_string(input,"|")){
            split_input_with_delimiters(input, &count_command, &value_command, "|");
            handle_pipe(count_command,value_command);
        }else if(check_characters_in_string(input,";")){
            split_input_with_delimiters(input, &count_command, &value_command, ";");
            if(count_command>4){
                printf("Maximum Supported argument is :{%d}.\n",MAX_ARGS);
            }else{
                handle_sequential(count_command,value_command);
            }
        }else if(check_characters_in_string(input,"~")){
            split_input_with_delimiters(input,&count_command, &value_command, "~");
            if(count_command>4){
                printf("Maximum Supported argument is :{%d}.\n",MAX_ARGS);
            }else{
                handle_concetination(count_command,value_command);   
            }           
        }else if(check_characters_in_string(input,"#")){
            split_input_with_delimiters(input,&count_command,&value_command," ");
            if(count_command>MAX_ARGS){
                printf("Maximum Supported argument is :{%d}.\n",MAX_ARGS);
            }else{
                handle_counting(count_command,value_command);  
            }
        }else if(check_characters_in_string(input,"+")){
            input[strlen(input) - 1] = '\0';  // Remove the '+'
            split_input_with_delimiters(input, &count_command, &value_command, " ");
            handle_background(count_command, value_command);
        }else if(strcmp(input,"fore")==0){
            printf("Handle fore...\n");
            handle_foreground();
        }else if(check_characters_in_string(input,"<")){
            printf("Input redirect: \n");
            split_input_with_delimiters(input, &count_command, &value_command, "<");
            handle_redirect(count_command, value_command, 1);
        }else if(check_characters_in_string(input,">>")){
            printf("Append Redirect. \n");
            split_input_with_delimiters(input, &count_command, &value_command, ">>");
            handle_redirect(count_command, value_command, 2);
        }else if(check_characters_in_string(input,">")){
            printf("Output redirect: \n");
            split_input_with_delimiters(input, &count_command, &value_command, ">");
            handle_redirect(count_command, value_command, 0);
        }
        else{
            split_input_with_delimiters(input, &count_command, &value_command, ";");
            if(count_command>4){
                printf("Maximum Supported argument is :{%d}.\n",MAX_ARGS);
            }else{
                handle_sequential(count_command,value_command);
            }
        }
    }
    return 0;
}

