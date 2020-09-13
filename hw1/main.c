#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#define MAX_INPUT_SZ 512
#define MAX_TOKEN_SZ 32
#define SPACE " "

// echo "Hi" > file : valid
// echo "Hi"> file : valid
// echo "Hi" >file : valid
void check_output_redirect(char **args)
{
    printf("%c", *args[0]);
}

char **process_input(char command[])
{
    char **args = malloc(MAX_TOKEN_SZ * sizeof(char *));
    args[0] = strtok(command, SPACE);
    int i = 0;
    while (args[i] != NULL)
    {
        args[++i] = strtok(NULL, SPACE);
    }
    return args;
}

void shell_loop()
{
    char command[MAX_INPUT_SZ]; //array of characters
    while (1)
    {
        char **c_args;
        printf("my_shell$ ");
        fflush(stdout);
        fgets(command, MAX_INPUT_SZ, stdin);
        command[strlen(command) - 1] = '\0'; //get rid of \n and replace with null
        c_args = process_input(command);
        execvp(c_args[0], c_args);
    }
}

int main(int argc, char *argv[])
{
    char cmd[11] = {'e', 'c', 'h', 'o', ' ', 'h', 'e', 'l', 'l', 'o', '\0'};
    check_output_redirect(process_input(cmd));
    return 0;
}