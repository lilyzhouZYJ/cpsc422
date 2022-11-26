#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <lib/string.h>
#include <file.h>

// Buffer for reading user input line
#define BUFLEN 1024
char input_buf[BUFLEN];

// Helper function:
// Get first element of command
void get_first_element(char * input, int input_start, int * start, int * end)
{
    printf("get_first_element with input %s, input_start %d\n", input, input_start);

    int start_idx = -1; // start index of the first element of input
    int input_len = strlen(input);

    for(int i = input_start; i <= input_len; i++){
        if(input[i] == ' ' || input[i] == '\0'){
            if(start_idx != -1){
                // Reached the end of the element
                *start = start_idx;
                *end = i;
                return;
            }
            // Otherwise just ignore the whitespace
        } else {
            if(start_idx == -1){
                // Reached the start of an element
                start_idx = i;
            }
        }
    }
}

// Helper function:
void get_first_non_arg_element(char * input, int * start, int * end)
{
    int input_start = 0;
    while(1){
        printf("get_first_non_arg_element: input is %s\n", input);

        // Get the first element
        *start = -1;
        *end = -1;
        get_first_element(input, input_start, start, end);

        printf("get_first_non_arg_element: start is %d, end is %d\n", *start, *end);

        if(start < 0 || start == end){
            // No more element
            return;
        }

        // Check if the fetched element does not start with '-'
        if(*(input + *start) != '-'){
            printf("get_first_non_arg_element: returning with start %d, end %d\n", *start, *end);
            return;
        }

        input_start = *end;
    }
}

int process_mkdir(char * args)
{
    printf("[d] IN PROCESS_MKDIR\n");

    // Get the first non-arg element
    int start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);

    if(start < 0 || start == end){
        printf("mkdir: missing operand\n");
        return -1;
    }

    // Copy into path
    char path[128];
    strncpy(path, args+start, end-start);
    path[end-start] = '\0';

    // Remove path from the args string
    args += end;

    printf("[d] process_mkdir: path is %s, remaining args is %s\n", path, args);

    return mkdir(path);
}

int process_cd(char * args)
{
    printf("[d] IN PROCESS_CD\n");

    if(args == NULL){
        printf("ls: args is null!\n");
        return -1;
    }

    // Get the first element of args
    int start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);

    // Get path
    char path[128];
    if(start == end || start < 0){
        // There is no argument to cd
        path[0] = '\0';
    } else {
        strncpy(path, args+start, end-start);
        path[end-start] = '\0';

        // Remove path from the args string
        args += end;
    }

    // printf("[d] process_cd: path is %s, remaining args is %s\n", path, args);

    return cd(path);
}

int process_pwd(char * args)
{
    printf("[d] IN PROCESS_PWD\n");
    return pwd();
}

int process_ls(char * args)
{
    printf("[d] IN PROCESS_LS\n");

    if(args == NULL){
        printf("ls: args is null!\n");
        return -1;
    }

    // Get the first element of args
    int start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);
    
    // Get path
    char path[128];
    if(start == end || start < 0){
        // There is no next element
        path[0] = '.';
        path[1] = '\0';
    } else {
        strncpy(path, args+start, end-start);
        path[end-start] = '\0';

        // Remove path from the args string
        args += end;
    }

    // printf("[d] process_ls: path is %s, remaining args is %s\n", path, args);

    // Open path as read-only
    int fd = open(path, O_RDONLY);
    if(fd < 0){
        printf("ls: could not open path %s\n", path);
        return -1;
    }

    // printf("[d] process_ls: opened file with fd %d\n", fd);

    // Fetch directory stats
    struct file_stat st;
    if(fstat(fd, &st) < 0){
        printf("ls: could not get file stat for path %s\n", path);
        close(fd);
        return -1;
    }

    // printf("[d] process_ls: fetched file stat\n", fd);

    // Make sure path is not a file
    if(st.type == T_FILE){
        // Path is a file; ls is invalid command
        printf("ls: path %s is a file\n", path);
        close(fd);
        return -1;
    }

    // Read all directory entries and print to console
    struct dirent de;
    int is_first_entry = 1;
    while(read(fd, (char*)&de, sizeof(de)) == sizeof(de)){
        // printf("[d] process_ls: read directory entry %s\n", de.name);
        if(de.inum != 0 && strcmp(de.name, ".") && strcmp(de.name, "..")){
            if(is_first_entry != 1){
                // not first entry; print with tab
                printf("\t%s", de.name);
            } else {
                // first entry; print without tab
                printf("%s", de.name);
                is_first_entry = 0;
            }
        }
    }
    printf("\n");

    close(fd);
    return 0;
}

/*
 * Process the given command.
 */
int process_command(const char * command, char * args)
{
    printf("[d] IN PROCESS_COMMAND with command %s (len %d)\n", command, strlen(command));

    if(command == NULL){
        printf("process_command: command is null!\n");
        return -1;
    }

    // Figure out what element we are looking at
    if(strcmp(command, "ls") == 0){
        printf("[d] FOUND LS\n");
        return process_ls(args);
    } else if (strcmp(command, "pwd") == 0){
        printf("[d] FOUND PWD\n");
        return process_pwd(args);
    } else if (strcmp(command, "cd") == 0){
        printf("[d] FOUND CD\n");
        return process_cd(args);
    } else if (strcmp(command, "mkdir") == 0){
        printf("[d] FOUND MKDIR\n");
        return process_mkdir(args);
    }

    printf("shell: not a valid command\n");
    return -1;
}

/*
 * Figure out what the command (first element) is given the user input.
 */
int process_input(char * input)
{
    printf("[d] IN PROCESS_INPUT with input %s\n", input);

    if(input == NULL){
        printf("process: input is null!\n");
        return -1;
    }

    int start = -1, end = -1;
    get_first_element(input, 0, &start, &end);
    if(start == -1 || end == -1){
        printf("shell: failed to parse command\n");
        return -1;
    }
    // printf("[d] process_input: fetched first element with start %d, end %d\n", start, end);

    // Copy command
    char command[end-start+1];
    strncpy(command, input+start, end-start);
    command[end-start] = '\0';

    input += end;

    // printf("[d] process_input: command is %s, remaining input is %s\n", command, input);

    return process_command(command, input);
}

int main(int argc, char **argv)
{
    printf("[d] shell started.\n");

    while(1){
        readline("$ ", input_buf);
        printf("[d] user input %s\n", input_buf);

        process_input(input_buf);
    }

    return 0;
}
