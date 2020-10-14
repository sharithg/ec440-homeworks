#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT_SZ 512
#define MAX_TOKEN_SZ 32
#define SPACE " "
#define INPUT_REDIRECT '>'
#define OUTPUT_REDIRECT '<'

typedef struct
{
    char meta;
    char **cmds;
} Metas;

int get_meta_size(Metas *metas)
{
    int i = 0;
    while (metas[i].meta != '\0')
    {
        i++;
    }
    return i;
}

void child_handler(int sig)
{
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
    }
}

Metas *get_token_struct(char **tokens)
{
    int i = 0;
    int metaIdx = 0;
    Metas *metas = malloc(10 * sizeof(*metas));
    int metaPtr = 0;
    while (tokens[i] != NULL)
    {
        if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], "<") == 0 || strcmp(tokens[i], "|") == 0 || strcmp(tokens[i], "&") == 0)
        {
            metas[metaIdx].meta = (char)(*tokens[i]);
            metas[metaIdx].cmds = malloc(MAX_TOKEN_SZ * sizeof(char *));
            int j = 0;
            while (j < i - metaPtr)
            {
                metas[metaIdx].cmds[j] = tokens[metaPtr + j];
                j++;
            }
            metaPtr = i + 1;
            metaIdx++;
        }
        i++;
    }

    metas[metaIdx].meta = 'n';
    int j = 0;
    metas[metaIdx].cmds = malloc(MAX_TOKEN_SZ * sizeof(char *));
    while (tokens[metaPtr] != NULL)
    {
        metas[metaIdx].cmds[j] = tokens[metaPtr];
        metaPtr++;
        j++;
    }
    return metas;
}

int runcmd(Metas *meta_struct)
{
    pid_t child_pid;
    int child_status;
    /////////////////////////////////////////////
    int foundInOut = 0;
    int ii = 0;
    char meta = 'n';
    char *file = NULL;
    char **cmds = NULL;
    int is_bg = 0;
    if (meta_struct[get_meta_size(meta_struct) - 2].meta != '&')
    {
        is_bg = 0;
        while (meta_struct[ii].meta != 'n')
        {
            if (meta_struct[ii].meta == '<')
            {
                meta = '<';
                file = meta_struct[ii + 1].cmds[0];
                cmds = meta_struct[ii].cmds;
                foundInOut = 1;
                if ((child_pid = fork()) == -1)
                    perror("Error");
            }
            else if (meta_struct[ii].meta == '>')
            {
                meta = '>';
                file = meta_struct[ii + 1].cmds[0];
                cmds = meta_struct[ii].cmds;
                foundInOut = 1;
                if ((child_pid = fork()) == -1)
                    perror("Error");
            }
            ii++;
        }
        if (foundInOut == 0)
        {
            // runcmd('n', meta_struct[0].cmds, NULL, 1, pipe_side);
            meta = 'n';
            file = NULL;
            cmds = meta_struct[0].cmds;
            if ((child_pid = fork()) == -1)
                perror("Error");
        }
    }
    else
    {
        is_bg = 1;
        int ii = 0;
        int foundInOut = 0;
        while (meta_struct[ii].meta != 'n')
        {
            if (meta_struct[ii].meta == '<')
            {
                meta = '<';
                file = meta_struct[ii + 1].cmds[0];
                cmds = meta_struct[ii].cmds;
                foundInOut = 1;
                if ((child_pid = fork()) == -1)
                    perror("Error");
            }
            else if (meta_struct[ii].meta == '>')
            {
                meta = '>';
                file = meta_struct[ii + 1].cmds[0];
                cmds = meta_struct[ii].cmds;
                foundInOut = 1;
                if ((child_pid = fork()) == -1)
                    perror("Error");
            }
            ii++;
        }
        if (foundInOut == 0)
        {
            meta = 'n';
            file = NULL;
            cmds = meta_struct[0].cmds;
            if ((child_pid = fork()) == -1)
                perror("Error");
        }
    }

    /////////////////////////////////////////////
    if (child_pid == 0)
    {

        if (meta == '>')
        {
            int fd1 = creat(file, 0644);
            dup2(fd1, STDOUT_FILENO);
            if (execvp(cmds[0], cmds) == -1)
            {
                perror("Error");
            }
            close(fd1);
        }
        else if (meta == '<')
        {
            int fd2 = open(file, 0644);
            dup2(fd2, STDIN_FILENO);
            if (execvp(cmds[0], cmds) == -1)
            {
                perror("Error");
            }
            close(fd2);
        }
        else
        {
            if (execvp(cmds[0], cmds) == -1)
            {
                perror("Error");
            }
        }
        exit(0);
    }
    else
    {
        if (is_bg == 1)
        {
            signal(SIGCHLD, child_handler);
        }
        else
        {
            pid_t tpid = wait(&child_status);
            do
            {

            } while (tpid != child_pid);
        }

        return child_status;
    }
    return 0;
}

void runpipe(char ***cmd, int isbg)
{
    int fd[2];
    pid_t pid;
    int fdd = 0;

    while (*cmd != NULL)
    {
        pipe(fd);
        if ((pid = fork()) == -1)
        {
            perror("Error");
            exit(1);
        }
        else if (pid == 0)
        {
            dup2(fdd, STDIN_FILENO);
            if (*(cmd + 1) != NULL)
            {
                dup2(fd[1], STDOUT_FILENO);
            }
            close(fd[0]);
            execvp((*cmd)[0], *cmd);
            exit(1);
        }
        else
        {
            if (isbg == 1)
            {
                signal(SIGCHLD, child_handler);
            }
            else
            {
                wait(NULL);
            }

            close(fd[1]);
            fdd = fd[0];
            cmd++;
        }
    }
}

char *clean_input(char command[])
{
    char *buffer = malloc(sizeof(*buffer) * MAX_INPUT_SZ);
    int i = 0;
    int buff = 0;
    while (command[i] != '\0')
    {
        switch (command[i])
        {
        case '>':
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
            break;
        }
        case '<':
        {
            if (command[i - 1] == ' ' && command[i + 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '<';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            else if (command[i + 1] == ' ' && command[i - 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '<';
                buffer[buff + 2] = ' ';
                i = i + 2;
                buff = buff + 3;
            }
            else if (command[i + 1] != ' ' && command[i - 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '<';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            else
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '<';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            break;
        }
        case '|':
        {
            if (command[i - 1] == ' ' && command[i + 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '|';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            else if (command[i + 1] == ' ' && command[i - 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '|';
                buffer[buff + 2] = ' ';
                i = i + 2;
                buff = buff + 3;
            }
            else if (command[i + 1] != ' ' && command[i - 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '|';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            else
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '|';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            break;
        }
        case '&':
        {
            if (command[i - 1] == ' ' && command[i + 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '&';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            else if (command[i + 1] == ' ' && command[i - 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '&';
                buffer[buff + 2] = ' ';
                i = i + 2;
                buff = buff + 3;
            }
            else if (command[i + 1] != ' ' && command[i - 1] != ' ')
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '&';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            else
            {
                buffer[buff] = ' ';
                buffer[buff + 1] = '&';
                buffer[buff + 2] = ' ';
                i = i + 1;
                buff = buff + 3;
            }
            break;
        }
        default:
        {
            buffer[buff] = command[i];
            i++;
            buff++;
            break;
        }
        }
    }
    return buffer;
}

char **split_tokens(char command[])
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

void shell_loop(int is_supressed)
{
    char command[MAX_INPUT_SZ]; //array of characters
    while (1)
    {
        char **c_args;
        if (is_supressed == 1)
            printf("");
        else
            printf("my_shell$ ");
        memset(command, '\0', MAX_INPUT_SZ);
        fflush(stdout);
        fgets(command, MAX_INPUT_SZ, stdin);
        if (command[0] == '\0')
        {
            exit(0);
            printf("\n");
        }
        command[strlen(command) - 1] = '\0'; //get rid of \n and replace with nu

        int i = 0;
        char *p = strtok(command, "|");
        char *array[50] = {NULL};

        while (p != NULL)
        {
            array[i++] = p;
            p = strtok(NULL, "|");
        }

        int jj = 0;
        while (array[jj] != NULL)
        {
            jj++;
        }
        if (jj > 1)
        {
            char **cmds[10];
            for (i = 0; i < jj; ++i)
            {
                char *cleaned1 = clean_input(array[i]);
                char **tokens1 = split_tokens(cleaned1);
                Metas *meta_struct1 = get_token_struct(tokens1);
                cmds[i] = meta_struct1[0].cmds;
            }
            if (command[strlen(command) - 2] == '&')
            {
                runpipe(cmds, 1);
            }
            else
            {
                runpipe(cmds, 0);
            }
        }
        else
        {

            char *total_cmd = clean_input(command);
            char **total_tokens = split_tokens(total_cmd);
            Metas *total_meta_struct = get_token_struct(total_tokens);
            runcmd(total_meta_struct);
        }
    }
}

int main(int argc, char *argv[])
{
    if ((argc > 1) && (strcmp("-n", argv[1]) == 0))
        shell_loop(1);
    else
        shell_loop(0);

    // char *ls[] = {"ls", NULL};
    // char *rev[] = {"wc", NULL};
    // char *nl[] = {"nl", NULL};
    // char *cat[] = {"cat", "-e", NULL};
    // char **cmd[] = {ls, rev, NULL};

    // pipeline(cmd);
    // return (0);

    // return 0;
}
