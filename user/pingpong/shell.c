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
void get_first_element(char * input, int * start, int * end)
{
    printf("IN GET_FIRST_ELEMENT with input %s\n", input);

    int start_idx = -1; // start index of the first element of input
    int input_len = strlen(input);

    for(int i = 0; i <= input_len; i++){
        if(input[i] == ' ' || input[i] == '\0'){
            if(start_idx != -1){
                // Reached the end of the element
                *start = start_idx;
                *end = i;
                printf("get_first_element: first element has start %d, end %d\n", *start, *end);
                return;
            }
            // Otherwise just ignore the whitespace
        } else {
            if(start_idx == -1){
                // Reached the start of an element
                start_idx = i;
            }
        }
        // Move pointer forward
        i++;
    }
}

int process_ls(char * args){

    printf("IN PROCESS_LS\n");

    if(args == NULL){
        printf("ls: args is null!\n");
        return -1;
    }

    // Get the first element of args
    int start = -1, end = -1;
    get_first_element(args, &start, &end);
    
    // Get path
    char * path = 0;
    if(start == end || start < 0){
        // There is no next element
        char path_tmp[] = ".";
        path = path_tmp;
    } else {
        char path_tmp[end-start+1];
        strncpy(path_tmp, args+start, end-start);
        path_tmp[end-start] = '\0';
        path = path_tmp;

        // Remove path from the args string
        args += end;
    }

    printf("process_ls: path is %s, remaining args is %s\n", path, args);

    // Open path as read-only
    int fd = open(path, O_RDONLY);
    if(fd < 0){
        printf("ls: could not open path %s\n", path);
        return -1;
    }

    // Fetch directory stats
    struct file_stat st;
    if(fstat(fd, &st) < 0){
        printf("ls: could not get file stat for path %s\n", path);
        close(fd);
        return -1;
    }

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
    printf("IN PROCESS_COMMAND\n");

    if(command == NULL){
        printf("process_command: command is null!\n");
        return -1;
    }

    // Figure out what element we are looking at
    if(strcmp(command, "ls") == 0){
        printf("FOUND LS\n");
        return process_ls(args);
    }

    printf("shell: not a valid command\n");
    return -1;
}

/*
 * Figure out what the command (first element) is given the user input.
 */
int process_input(char * input)
{
    printf("IN PROCESS_INPUT with input %s\n", input);

    if(input == NULL){
        printf("process: input is null!\n");
        return -1;
    }

    int start = -1, end = -1;
    get_first_element(input, &start, &end);
    if(start == -1 || end == -1){
        printf("shell: failed to parse command\n");
        return -1;
    }
    printf("process_input: fetched first element with start %d, end %d\n", start, end);

    // Copy command
    char command[end-start+1];
    strncpy(command, input+start, end-start);
    command[end-start] = '\0';

    input += end;

    printf("process_input: command is %s, remaining input is %s\n", command, input);

    return process_command(command, input);
}

int main(int argc, char **argv)
{
    printf("shell started.\n");

    while(1){
        readline("$ ", input_buf);
        printf("user input %s\n", input_buf);

        process_input(input_buf);
    }

    return 0;
}
