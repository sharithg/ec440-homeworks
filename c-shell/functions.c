#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_INPUT_SZ 512
#define MAX_TOKEN_SZ 32
#define SPACE " "
#define INPUT_REDIRECT '>'
#define OUTPUT_REDIRECT '<'

struct Metas
{
    char *meta[2];
    char *author[MAX_TOKEN_SZ];
};

char *clean_input(char command[])
{
    char *buffer = malloc(sizeof(*buffer) * MAX_INPUT_SZ);
    int i = 0;
    int buff = 0;
    while (command[i] != '\0')
    {
        if (command[i] == '>')
        {
            if (command[i - 1] == ' ' && command[i + 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '>';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            else if (command[i + 1] == ' ' && command[i - 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '>';
                buffer[buff + 2] = ' ';
                i = i + 2;
                buff = buff + 3;
            }
            else if (command[i + 1] != ' ' && command[i - 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '>';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            else
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '>';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
        }
        else
        {
            buffer[buff] = command[i];
            i++;
            buff++;
        }
    }
    return buffer;
}

char **split_tokens(char command[])
{
    char **argv = malloc(MAX_TOKEN_SZ * sizeof(char *));

    char cmds[strlen(command)];
    memcpy(cmds, command, strlen(command));
    int i = 0;
    char *piece = strtok(cmds, " ");
    while (piece != NULL)
    {
        argv[i++] = piece;
        piece = strtok(NULL, " ");
    }
    argv[i] = NULL;
    return argv;
}