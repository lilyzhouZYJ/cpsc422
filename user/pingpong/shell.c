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

    int is_string = 0;

    for(int i = input_start; i <= input_len; i++){
        if(input[i] == ' ' || input[i] == '\0' || input[i] == '\b' || input[i] == '\x7f'){
            if(is_string == 0 && start_idx != -1){
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

                if(input[i] == '"'){
                    // We are looking for a closing quotation mark
                    is_string = 1;
                }
            } else {
                if(input[i] == '"' && is_string == 1){
                    // Found closing quotation mark
                    *start = start_idx;
                    *end = i + 1;
                    return;
                }
            }
        }
    }
}

// Helper function:
void get_first_non_arg_element(char * input, int * start, int * end)
{
    int input_start = 0;
    while(1){
        // printf("get_first_non_arg_element: input is %s\n", input);

        // Get the first element
        *start = -1;
        *end = -1;
        get_first_element(input, input_start, start, end);

        // printf("get_first_non_arg_element: start is %d, end is %d\n", *start, *end);

        if(start < 0 || start == end){
            // No more element
            return;
        }

        // Check if the fetched element does not start with '-'
        if(*(input + *start) != '-'){
            // printf("get_first_non_arg_element: returning with start %d, end %d\n", *start, *end);
            return;
        }

        input_start = *end;
    }
}

// Helper function:
// Check if the -r flag exists in input
int includes_recursion(char * input)
{
    printf("[d] includes_recursion: input is %s\n", input);
    int input_start = 0;
    while(1){
        int start = -1, end = -1;
        get_first_element(input, input_start, &start, &end);

        if(start < 0 || start == end){
            // No more elements
            return 0;
        }

        // Check if the fetched element is '-r'
        char element[end-start+1];
        element[end-start] = '\0';
        strncpy(element, input+start, end-start);
        // printf("start %d, end %d, element %s\n", start, end, element);
        if(strcmp(element, "-r") == 0){
            // found -r flag
            return 1;
        }

        input_start = end;
    }

    return 0;
}

void join_path(char * path1, char * path2, char * buffer){
    printf("join path: %s and %s ", path1, path2);

    int len1 = strlen(path1);
    if(*path2 == '/'){
        path2++;
    }
    int len2 = strlen(path2);

    strncpy(buffer, path1, len1);
    int buf_size = len1;

    // Check if we need to manually add /
    if(buffer[len1-1] != '/'){
        buffer[len1] = '/';
        buf_size++;
    }

    strncpy(buffer+buf_size, path2, len2);
    buf_size += len2;

    buffer[buf_size] = '\0';

    printf("result %s\n", buffer);
}

// Helper: get only the last-level name from a path
void get_last_name_from_path(char * path, char * buffer)
{
    int path_len = strlen(path);
    int start = 0, end = path_len;

    // If path ends with /, remove it
    if(path[path_len-1] == '/'){
        path_len--;
        end = path_len - 1;
    }

    for(int i = end-1; i >= 0; i--){
        if(path[i] == '/'){
            // found start
            start = i + 1;
            break;
        }
    }

    strncpy(buffer, path+start, end-start);
    buffer[end-start] = '\0';

    printf("get_name_from_path: path is %s, last elt is %s\n", path, buffer);
}

// Helper
int non_recursive_cp(char * src, char * dest)
{
    printf("IN NON-RECURSIVE_CP with src %s, dest %s\n", src, dest);
    
    // (a) Open src
    int src_fd = open(src, O_RDONLY);
    if(src_fd < 0){
        printf("cp: cannot open src %s\n", src);
        return -1;
    }

    // (b) Try to open dest
    int dest_fd = open(dest, O_CREATE);
    if(dest_fd < 0){
        // this may be due to dest being a directory;
        // try to open as read-only
        dest_fd = open(dest, O_RDONLY);
        if(dest_fd < 0){
            // dest does not exist
            printf("cp: cannot open dest %s\n", dest);
            return -1;
        }

        // dest is a directory; 
        // need to create a file in it with same name as src
        int dest_len = strlen(dest);
        if(dest[dest_len-1] == '/'){
            dest[dest_len-1] = '\0';
            dest_len--;
        }
        char src_name[128];
        get_last_name_from_path(src, src_name);
        int src_len = strlen(src_name);
        printf("[d] process_cp: src_name %s (%d), dest %s (%d)\n", src_name, src_len, dest, dest_len);
        char dest_file[dest_len + src_len + 2];
        dest_file[dest_len + src_len + 1] = '\0';
        strncpy(dest_file, dest, dest_len);
        dest_file[dest_len] = '/';
        strncpy(dest_file+dest_len+1, src_name, src_len);

        printf("[d] process_cp: dest is existing directory, so create %s\n", dest_file);

        dest_fd = open(dest_file, O_CREATE | O_WRONLY);
        if(dest_fd < 0){
            printf("cp: error creating dest\n");
            return -1;
        }
    } else {
        // Open succeeded did not return -1, so dest is a file;
        // re-open as writable
        close(dest_fd);
        dest_fd = open(dest, O_WRONLY);
    }

    // (c) Double check dest is a file now
    struct file_stat dest_stat;
    if(fstat(dest_fd, &dest_stat) < 0){
        printf("cp: error fetching stat for dest %s\n", dest);
        return -1;
    }
    if(dest_stat.type != T_FILE){
        printf("cp: dest file is not a file\n");
        return -1;
    }

    // (d) Copy from src to dest file
    char buffer[9000];
    int n_read;
    while((n_read = read(src_fd, buffer, 9000)) > 0){
        if(write(dest_fd, buffer, n_read) < n_read){
            printf("cp: error in copying from src to dest\n");
            return -1;
        }
    }

    return 0;
}

// // Helper: make sure dest is not a subdirectory of src
// int is_dest_subdir_of_src(char * src, char * dest)
// {
//     int dest_fd = open(dest, O_RDONLY);
//     if(dest_fd < 0){
//         return -1;
//     }

//     // Follow .. to find each ancestor
//     struct dirent de;
//     while(read(dest_fd, (char*)&de, sizeof(de)) == sizeof(de)){
//         if(de.inum != 0 && strcmp(de.name, "..")){
//             // Found parent
//         }
//     }
// }

// Helper
int recursive_cp(char * src, char * dest)
{
    printf("IN RECURSIVE_CP with src %s, dest %s\n", src, dest);

    // (1) Open src
    int src_fd = open(src, O_RDONLY);
    if(src_fd < 0){
        printf("cp: cannot open src %s\n", src);
        return -1;
    }

    // (2) Check src file type
    struct file_stat src_stat;
    if(fstat(src_fd, &src_stat) < 0){
        close(src_fd);
        printf("cp: error fetching stat for src %s\n", src);
        return -1;
    }
    // If src is a file, go to non-recursive case
    if(src_stat.type == T_FILE){
        close(src_fd);
        return non_recursive_cp(src, dest);
    }

    // (b) Try to open dest
    int dest_fd = open(dest, O_CREATE);
    if(dest_fd < 0){
        // unclear if dest exists; 
        // try to open as read-only to check if it is existing directory
        dest_fd = open(dest, O_RDONLY);
        if(dest_fd < 0){
            // dest does not exist;
            // try to create it
            if(mkdir(dest) < 0){
                printf("cp: cannot create dest %s\n", dest);
                close(src_fd);
                return -1;
            }
            dest_fd = open(dest, O_RDONLY);
        }
    }

    // (c) Double check dest is a directory
    if(dest_fd < 0){
        close(src_fd);
        printf("cp: cannot open dest\n");
        return -1;
    }
    struct file_stat dest_stat;
    if(fstat(dest_fd, &dest_stat) < 0){
        close(src_fd);
        close(dest_fd);
        printf("cp: error fetching stat for dest %s\n", dest);
        return -1;
    }
    if(dest_stat.type != T_DIR){
        close(src_fd);
        close(dest_fd);
        printf("cp: dest is not a directory\n");
        return -1;
    }

    // (d) Copy all files and subdirectories of src into dest
    struct dirent de;
    while(read(src_fd, (char*)&de, sizeof(de)) == sizeof(de)){
        // printf("[d] recursive_cp: read directory entry %s\n", de.name);
        if(de.inum != 0 && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0){
            if(recursive_cp(de.name, dest) < 0){
                printf("cp: failed to copy %s to %s\n", de.name, dest);
                close(src_fd);
                close(dest_fd);
                return -1;
            }
        }
    }

    close(src_fd);
    close(dest_fd);
    return 0;
}

int process_cp(char * args)
{
    printf("[d] IN PROCESS_CP with args %s\n", args);

    // Check if this is recursive
    int is_recursive = includes_recursion(args);

    // Fetch src and dest
    int start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);
    if(start < 0 || start == end){
        printf("cp: missing source operand\n");
        return -1;
    }

    char src[end-start+1];
    strncpy(src, args+start, end-start);
    src[end-start] = '\0';
    args += end;

    start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);
    if(start < 0 || start == end){
        printf("cp: missing destination operand\n");
        return -1;
    }

    char dest[end-start+1];
    strncpy(dest, args+start, end-start);
    dest[end-start] = '\0';

    printf("[d] process_cp: src %s, dest %s, recursive %d\n", src, dest, is_recursive);

    // Open src
    int src_fd = open(src, O_RDONLY);
    if(src_fd < 0){
        printf("cp: cannot open src %s\n", src);
        return -1;
    }

    // Get src file type
    struct file_stat src_stat;
    if(fstat(src_fd, &src_stat) < 0){
        printf("cp: error fetching stat for src %s\n", src);
        close(src_fd);
        return -1;
    }

    close(src_fd);
    
    if(src_stat.type == T_FILE){
        // If src is a file, go to non-recursive case
        return non_recursive_cp(src, dest);
    } else {
        // Src is directory; must be recursive
        if(is_recursive == 0){
            printf("cp: src %s is a directory, but no recursive flag is given\n", src);
            return -1;
        }
        // Go to recursive case
        return recursive_cp(src, dest);
    }
}

// Helper
int recursive_rm(char * path)
{
    printf("IN RECURSIVE_RM with path %s\n", path);

    // (1) Open path
    int path_fd = open(path, O_RDONLY);
    if(path_fd < 0){
        printf("rm: cannot open path %s\n", path);
        return -1;
    }

    // (2) Check path file type
    struct file_stat path_stat;
    if(fstat(path_fd, &path_stat) < 0){
        close(path_fd);
        printf("rm: error fetching stat for path %s\n", path);
        return -1;
    }
    // If path is a file, this is non-recursive: just unlink it
    if(path_stat.type == T_FILE){
        printf("[d] recursive_rm: path %s is file\n", path);
        close(path_fd);
        return unlink(path);
    }

    // Remove all files + subdirectories of path
    struct dirent de;
    char buffer[128];
    while(read(path_fd, (char*)&de, sizeof(de)) == sizeof(de)){
        printf("[d] recursive_rm: read directory entry %s, %d\n", de.name, de.inum);
        if(de.inum != 0 && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0){
            join_path(path, de.name, buffer);

            if(recursive_rm(buffer) < 0){
                printf("rm: failed to remove %s\n", de.name);
                close(path_fd);
                return -1;
            }
        }
    }

    close(path_fd);
    unlink(path);
    return 0;
}

int process_rm(char * args)
{
    printf("[d] IN PROCESS_RM with args %s\n", args);

    // Check if this is recursive
    int is_recursive = includes_recursion(args);

    // Fetch path
    int start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);
    if(start < 0 || start == end){
        printf("rm: missing operand\n");
        return -1;
    }

    char path[end-start+1];
    strncpy(path, args+start, end-start);
    path[end-start] = '\0';

    printf("[d] process_rm: path %s, recursive %d\n", path, is_recursive);

    // Open path
    int path_fd = open(path, O_RDONLY);
    if(path_fd < 0){
        printf("cp: cannot find path %s\n", path);
        return -1;
    }

    // Get path file type
    struct file_stat path_stat;
    if(fstat(path_fd, &path_stat) < 0){
        printf("rm: error fetching stat for path %s\n", path);
        close(path_fd);
        return -1;
    }

    close(path_fd);
    
    if(path_stat.type == T_FILE){
        // If path is a file, just unlink it
        unlink(path);
    } else {
        // Path is directory; must be recursive
        if(is_recursive == 0){
            printf("rm: path %s is a directory, but no recursive flag is given\n", path);
            return -1;
        }
        // Go to recursive case
        return recursive_rm(path);
    }

    return 0;
}

int process_mv(char * args)
{
    printf("[d] IN PROCESS_MV with args %s\n", args);

    // process_cp + process_rm
    if(process_cp(args) < 0){
        printf("mv: error\n");
        return -1;
    }
    if(process_rm(args) < 0){
        printf("mv: error\n");
        return -1;
    }
    return 0;
}

int process_redir(char * string, char * args)
{
    // Process string: make sure to remove " at the front and end 
    string++;   
    int string_len = strlen(string);
    string[string_len-1] = '\0';
    string_len--;

    // Get the non-arg elements
    int start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);
    if(start < 0 || start == end){
        printf("redir: missing operand\n");
        return -1;
    }

    char op[end-start+1];
    strncpy(op, args+start, end-start);
    op[end-start] = '\0';

    args += end;

    start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);
    if(start < 0 || start == end){
        printf("redir: missing operand\n");
        return -1;
    }

    char path[end-start+1];
    strncpy(path, args+start, end-start);
    path[end-start] = '\0';

    printf("redir: string is %s, op is %s, file is %s\n", string, op, path);

    // Determine if we are writing or appending
    int is_append = 0;
    if(strcmp(op, ">>") == 0){
        is_append = 1;
    } else if (strcmp(op, ">") == 0){
        is_append = 0;
    } else {
        printf("redir: expecting > or >> as second argument\n");
        return -1;
    }

    // Open file
    int fd = open(path, O_RDWR);
    if(fd < 0){
        printf("redir: cannot open file\n");
        return -1;
    }

    // Make sure path is a file
    struct file_stat stat;
    if(fstat(fd, &stat) < 0){
        printf("redir: cannot get file stats\n");
        return -1;
    }
    if(stat.type != T_FILE){
        printf("redir: %s is not a file\n", path);
        return -1;
    }

    if(is_append == 1){
        // Append to file
        char buf[900];
        while (read(fd, buf, 900) > 0) {}
        if(write(fd, string, string_len) != string_len){
            printf("redir: error in append\n");
            return -1;
        }
    } else {
        // Write to file
        if(write(fd, string, string_len) != string_len){
            printf("redir: error in write\n");
            return -1;
        }
    }

    return 0;
}

int process_cat(char * args)
{
    printf("[d] IN PROCESS_CAT\n");

    // Get the first non-arg element
    int start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);

    if(start < 0 || start == end){
        printf("cat: missing operand\n");
        return -1;
    }

    // Copy into path
    char path[128];
    strncpy(path, args+start, end-start);
    path[end-start] = '\0';

    // Open file
    int fd = open(path, O_RDONLY);
    if(fd < 0){
        printf("cat: cannot open file %s\n", path);
        return -1;
    }

    // Read and print
    char buffer[9000];
    int n_read;
    while((n_read = read(fd, buffer, 9000)) > 0){
        printf(buffer);
    }
    printf("\n");

    if(n_read < 0){
        printf("cat: error in reading\n");
        return -1;
    }

    return 0;
}

int process_touch(char * args)
{
    printf("[d] IN PROCESS_TOUCH\n");

    // Get the first non-arg element
    int start = -1, end = -1;
    get_first_non_arg_element(args, &start, &end);

    if(start < 0 || start == end){
        printf("touch: missing operand\n");
        return -1;
    }

    // Copy into path
    char path[128];
    strncpy(path, args+start, end-start);
    path[end-start] = '\0';

    int fd = open(path, O_CREATE);
    if(fd < 0){
        printf("touch: failed to create file\n");
        return -1;
    }

    return 0;
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

    // printf("[d] process_mkdir: path is %s, remaining args is %s\n", path, args);

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

    if(chdir(path) < 0){
        printf("cd: could not change directory to %s\n", path);
        return -1;
    } else {
        return 0;
    }
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
                printf(" %s", de.name);
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
int process_command(char * command, char * args)
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
    } else if (strcmp(command, "cp") == 0){
        printf("[d] FOUND CP\n");
        return process_cp(args);
    } else if (strcmp(command, "rm") == 0){
        printf("[d] FOUND RM\n");
        return process_rm(args);
    } else if (strcmp(command, "mv") == 0){
        printf("[d] FOUND MV\n");
        return process_mv(args);
    } else if (strcmp(command, "touch") == 0){
        printf("[d] FOUND TOUCH\n");
        return process_touch(args);
    } else if (strcmp(command, "cat") == 0){
        printf("[d] FOUND CAT\n");
        return process_cat(args);
    } else if (*command == '"'){
        printf("[d] FOUND REDIR\n");
        return process_redir(command, args);
    }

    printf("shell: not a valid command\n");
    return -1;
}

/*
 * Figure out what the command (first element) is given the user input.
 */
int process_input(char * input)
{
    // printf("[d] IN PROCESS_INPUT with input %s\n", input);

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
    printf("[d] process_input: fetched first element with start %d, end %d\n", start, end);

    // Copy command
    char command[end-start+1];
    strncpy(command, input+start, end-start);
    command[end-start] = '\0';

    input += end;

    printf("[d] process_input: command is %s with len %d, remaining input is %s\n", command, strlen(command), input);

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
