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

void prepare_pipes()
{
}

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

    /* EEEEXTEERMINAAATE! */
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

int runcmd(char meta, char **argv, char *file, int is_bg, int pipe_side, int fd[])
{
    printf("FD %d %d\n", fd[1], fd[2]);
    pid_t child_pid;
    int child_status;
    child_pid = fork();
    int camden = 5;
    if (child_pid == 0)
    {
        if (meta == '>')
        {
            int fd1 = creat(file, 0644);
            dup2(fd1, STDOUT_FILENO);
            execvp(argv[0], argv);
            close(fd1);
        }
        else if (meta == '<')
        {
            int fd2 = open(file, 0644);
            dup2(fd2, STDIN_FILENO);
            execvp(argv[0], argv);
            close(fd2);
        }
        else if (pipe_side == 1)
        {
            printf("In right pipe (%s)\n", argv[0]);
            close(fd[1]);
            close(STDIN_FILENO);
            dup(fd[0]);
            close(fd[0]);
            execvp(argv[0], argv);
        }
        else if (pipe_side == 0)
        {
            printf("In left pipe (%s)\n", argv[0]);
            close(fd[0]);
            close(STDOUT_FILENO);
            dup(fd[1]);
            close(fd[1]);
            execvp(argv[0], argv);
        }
        else
        {
            camden = 6;
            execvp(argv[0], argv);
        }
        /* This is done by the child process. */
        // printf("here1 %s\n", file);

        /* If execvp returns, it must have failed. */

        // printf("Unknown command\n");
        exit(0);
    }
    else
    {
        /* This is run by the parent.  Wait for the child
        to terminate. */
        if (is_bg == 0)
        {
            if (pipe_side == 0)
            {
            }

            pid_t tpid = wait(&child_status);
            do
            {

                if (tpid != child_pid)
                    printf("done");
            } while (tpid != child_pid);

            return child_status;
        }
        else
        {
            struct sigaction sa;
            printf("HERE\n");
            sa.sa_handler = child_handler;
            if (sigaction(SIGCHLD, &sa, NULL))
            {
                perror("sigaction");
                return 1;
            }
            return child_status;
        }
    }
    printf("candem: %d\n", camden);
    return 0;
}

void run_all_cmds(Metas *meta_struct, int pipe_side, int fd[])
{
    if (meta_struct[get_meta_size(meta_struct) - 2].meta == '&')
    {
        int ii = 0;
        int foundInOut = 0;
        while (meta_struct[ii].meta != 'n')
        {
            if (meta_struct[ii].meta == '<')
            {
                // printf("%s %s", meta_struct[ii].cmds[0], meta_struct[ii + 1].cmds[0]);
                runcmd('<', meta_struct[ii].cmds, meta_struct[ii + 1].cmds[0], 1, pipe_side, fd);
                foundInOut = 1;
            }
            else if (meta_struct[ii].meta == '>')
            {
                runcmd('>', meta_struct[ii].cmds, meta_struct[ii + 1].cmds[0], 1, pipe_side, fd);
                foundInOut = 1;
            }
            ii++;
        }
        if (foundInOut == 0)
        {
            runcmd('n', meta_struct[0].cmds, NULL, 1, pipe_side, fd);
        }
    }
    else
    {
        int ii = 0;
        int foundInOut = 0;
        while (meta_struct[ii].meta != 'n')
        {
            if (meta_struct[ii].meta == '<')
            {
                // printf("%s %s", meta_struct[ii].cmds[0], meta_struct[ii + 1].cmds[0]);
                runcmd('<', meta_struct[ii].cmds, meta_struct[ii + 1].cmds[0], 0, pipe_side, fd);
                foundInOut = 1;
            }
            else if (meta_struct[ii].meta == '>')
            {
                runcmd('>', meta_struct[ii].cmds, meta_struct[ii + 1].cmds[0], 0, pipe_side, fd);
                foundInOut = 1;
            }
            ii++;
        }
        if (foundInOut == 0)
        {
            runcmd('n', meta_struct[0].cmds, NULL, 0, pipe_side, fd);
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

void shell_loop()
{
    char command[MAX_INPUT_SZ]; //array of characters
    while (1)
    {
        char **c_args;
        printf("my_shell$ ");
        fflush(stdout);
        fgets(command, MAX_INPUT_SZ, stdin);
        command[strlen(command) - 1] = '\0'; //get rid of \n and replace with nu
        int i = 0;
        char pipe1[100];
        char pipe2[100];
        while (command[i] != '\0')
        {
            if (command[i] == '|')
            {
                int ii = 0;
                int jj = 0;
                while (ii < i)
                {
                    pipe1[ii] = command[ii];
                    ii++;
                }
                while (jj < strlen(command))
                {
                    pipe2[jj] = command[jj + i + 1];
                    jj++;
                }
            }
            i++;
        }
        char *cleaned1 = clean_input(pipe1);
        char *cleaned2 = clean_input(pipe2);
        char **tokens1 = split_tokens(cleaned1);
        char **tokens2 = split_tokens(cleaned2);
        Metas *meta_struct1 = get_token_struct(tokens1);
        Metas *meta_struct2 = get_token_struct(tokens2);
        // setup pipe
        int fd[2];
        pipe(&fd[0]);
        run_all_cmds(meta_struct1, 0, fd);
        run_all_cmds(meta_struct2, 1, fd);
        //
        //Backgound process
        // run_all_cmds(meta_struct);
        // while (meta_struct[i].meta != '\0')
        // {
        //     int j = 0;
        //     printf("Char (%c): ", meta_struct[i].meta);
        //     while (meta_struct[i].cmds[j] != NULL)
        //     {
        //         printf(" %s", meta_struct[i].cmds[j]);
        //         j++;
        //     }
        //     i++;
        //     printf("\n");
        // }

        // runcmd('&', meta_struct[0].cmds, "file");
        // printf("Run: %s\n", meta_struct[0].cmds[1]);
        // int i = 0;
        // while (meta_struct[i].meta != '\0')
        // {
        //     int j = 0;
        //     printf("Char (%c): ", meta_struct[i].meta);
        //     while (meta_struct[i].cmds[j] != NULL)
        //     {
        //         printf(" %s", meta_struct[i].cmds[j]);
        //         j++;
        //     }
        //     i++;
        //     printf("\n");
        // }

        // int ii = 0;
        // while (meta_struct[ii].meta != 'n')
        // {
        //     if (meta_struct[ii].meta == '<')
        //     {
        //         // printf("%s %s", meta_struct[ii].cmds[0], meta_struct[ii + 1].cmds[0]);
        //         runcmd('<', meta_struct[ii].cmds, meta_struct[ii + 1].cmds[0]);
        //     }
        //     else if (meta_struct[ii].meta == '>')
        //     {
        //         runcmd('>', meta_struct[ii].cmds, meta_struct[ii + 1].cmds[0]);
        //     }
        //     ii++;
        // }
    }
}

int main(int argc, char *argv[])
{
    shell_loop();

    return 0;
}
