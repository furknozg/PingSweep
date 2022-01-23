#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <pthread.h>

/// AUTHOR: Furkan Özgültekin
/// NOTES: Most of this project is a subset of a school project we worked on with my friend called Shellington (A commandline user interface with our own built in custom commands)
/// Most of that projects code was a template provided to ourselves so I decided not to include it to avoid any plagiarsim the enumerations are the only part of the project cropped from the original
/// in which I have not written.

enum return_codes
{
    SUCCESS = 0,
    EXIT = 1,
    UNKNOWN = 2,
};

struct thread_args
{
    const char * subnet;
    const char * range_start;
    const char * range_end;
};

const char *sysname = "bash";

int file_exists(const char *path_name)
{
    // opens the path directory to read file path which if exists:
    // 1. the pointer will not be null
    // 2. under the condition of one will return true since path to file exists and is valid
    FILE *fp;

    if ((fp = fopen(path_name, "r")))
    {
        fclose(fp);
        return 1;
    }

    return 0;
}

char *search_path(const char *file_name)
{
    // is intended to traverse the existing shell path environment var
    // $PATH to get the individual file path tokens to scan them all
    char *env = getenv("PATH");
    // for the $PATH var the outputs delimiter is ":", for unix based devices
    const char delim[2] = ":";
    char *paths_env = strtok(env, delim);

    //initializing the path array to check if the concatenation of the desired file_name to it exists
    const int max_len = 30;

    while (paths_env != NULL)
    {
        // Intention is to search for the path in a manner where $PATH[i]/path
        // is an existing file meaning that it's the file at question to be executed
        char *path = malloc(sizeof(paths_env) + max_len);
        if (path == NULL)
        {
            //in case of a failed malloc
            return NULL;
        }
        path[max_len] = NULL;
        strcpy(path, paths_env);
        strcat(path, "/");
        strcat(path, file_name);

        if (file_exists(path))
        {
            return path;
        }
        paths_env = strtok(NULL, delim);
        free(path);
    }
    return NULL;
}

int ping_sweep(const char *subnet, const char *range_start, const char *range_end)
{
    /// AUTHOR: Furkan Özgültekin

    int stat;

    // the standard ping script used in linux devices is used to scan all desired ports in the interval
    char *file_path = search_path("ping");
    if (file_path == NULL)
    {
        printf("-%s: %s: command not found\n", sysname, "ping");
        return -1;
    }

    // use grep path if it exists

    // usage: pingsweep subnet range_start range_end
    // where range is between (0,254) by default
    int i = atoi(range_start);
    int end = atoi(range_end);

    if (i > 254 || end > 254 || i < 0 || end < 0)
    {
        // the subnet consists of 255 individual ip's eg: 192.168.0.0 to 192.168.0.254
        printf("Invalid range operators");
        return 0;
    }

    pid_t pid_main = fork();
    if (pid_main == 0)
    {
        // create child processes and then execute ping for the interval between start and end using for
        for (i; i <= end; i++)
        {

            // count is also the digit count of the said current_index so there is an if else statement to change it accordingly
            int count = 1;
            if (i >= 10)
            {
                count = 2;
            }
            else if (i >= 100)
            {
                count = 3;
            }
            char cur_index[count];
            sprintf(cur_index, "%d", i);

            char *ping_net = malloc(sizeof(subnet) + sizeof(cur_index) + 2);

            // the concats are used to get the desired argument pass of $(SUBNET).$(CUR_INDEX)
            strcat(ping_net, subnet);
            strcat(ping_net, ".");
            strcat(ping_net, cur_index);

            // the -c 1 argument is to transmit only one ping for all subnets we want to peek
            char *const args[] = {"ping", "-c 1", ping_net, NULL};

            // each ping becomes a child process of their own
            pid_t pid = fork();
            if (pid == 0)
            {
                execv(file_path, args);
            }
            else
            {
                wait(NULL);
            }
            free(ping_net);
        }
    }
    else
    {
        printf("Scanning on subnet %s from %s to %s\n", subnet, range_start, range_end);
        wait(&stat);
        return SUCCESS;
    }
}

int main(const int argc, const char **argv)
{
    if (argc - 1 > 3)
    {
        if (strcmp(argv[4], "-t") == 0)
        {
            if (argc == 6)
            {
                int n = atoi(argv[5]);
                pthread_t tid[n];
                for (int i = 0; i < n; i++)
                {
                    struct thread_args *args = malloc (sizeof (struct thread_args));
                    args-> subnet = argv[1];
                    args->range_start = argv[2] + n*i;
                    args->range_end = argv[3] + n*(i + 1);
                    pthread_create(&tid, NULL, ping_sweep, args);
                }
            }
        }
        else
        {
            printf("Usage: ./outfile subnet lower upper [optional: -t thread_count]\n");
        }
        exit(1);
    }

    if (argc - 1 == 3)
    {
        ping_sweep(argv[1], argv[2], argv[3]);
    }
    else
    {
        printf("Usage: ./outfile subnet lower upper [optional: -t thread_count]\n");
        exit(1);
    }

    return 0;
}