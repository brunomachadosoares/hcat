/* vi: set sw=4 ts=4: */
/*
 *
 * hcat - Add data at the beggining of the file.
 *
 * Copyright 2009 - Bruno M. Soares <bm.soares@hotmail.com>
 *
 * Licensed under GPLv2, see file LICENSE for details.
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <unistd.h>

#define VERSION     "1.3"
#define STDIN       0

#define WRITE       "w"
#define READ        "r"
#define APPEND      "a"

#define BEGIN       1
#define END         0


FILE *fopen_or_warn(char *path, const char *flag)
{
    FILE *fd = NULL;

    if( (path == NULL) || (flag == NULL) ) {
        return NULL;
    }

    if((fd = fopen(path, flag)) != NULL) {
        return fd;
    } else {
        fprintf(stderr, "Fail to open [%s]\n", path);
        exit(1);
    }
}

int show_help(char *process)
{
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Use:\n");
    fprintf(stderr, "<data> |%s <file dst>\n\n", process);

    return -1;
}

int show_version(void)
{
    fprintf(stderr, "Version: %s\n", VERSION);

    return 0;
}

void write_buff_by_line(int num, char *to, int flag, GList *buff)
{
    int i       = 0;
    int length  = 0;
    char *line  = NULL;
    FILE *fd_to = NULL;
        
    length = g_list_length(buff);

    if(num >= length) {
        fprintf(stderr, "Line number is biggest than file length.\n");
        exit(1);
    }

    if(flag == BEGIN) {
        fd_to = fopen_or_warn(to, WRITE);

        for(i=0; i < num; i++) {
            line = g_list_nth_data(buff, i);
            if(line != NULL) {
                fprintf(fd_to, "%s\n", line);
            }
        }

    }

    if(flag == END) {
        fd_to = fopen_or_warn(to, APPEND);

        for(i=num; i < length; i++) {
            line = g_list_nth_data(buff, i);
            if(line != NULL) {
                fprintf(fd_to, "%s\n", line);
            }
        }
    }

    fclose(fd_to);
    g_free(line);
}

void write_buff(char *to, GList *buff)
{
    int i       = 0;
    int length  = 0;
    char *line  = NULL;
    FILE *fd_to = NULL;

    fd_to = fopen_or_warn(to, APPEND);

    length = g_list_length(buff);

    for(i=0; i < length; i++) {
        line = g_list_nth_data(buff, i);
        if(line != NULL) {
            fprintf(fd_to, "%s\n", line);
        }
    }

    fclose(fd_to);
}

GList *store_file(char *file)
{
    unsigned int len = 0;
    char *line       = NULL;
    FILE *arq        = NULL;
    GList *buff      = NULL;

    arq = fopen_or_warn(file, READ);

    while(getline(&line, &len, arq) > 0) {
        line[strlen(line) -1] = '\0';
        buff = g_list_append(buff, g_strdup(line));
    }

    fclose(arq);
    g_free(line);

    return buff;
}

int called_by_pipe(char *to)
{
    int i            = 0;
    int length       = 0;
    unsigned int len = 0;
    char *line       = NULL;
    FILE *fd_to      = NULL;
    GList *file_buff = NULL;
    GList *buff      = NULL;
    struct pollfd      ufds;

    ufds.fd     = STDIN;
    ufds.events = POLLIN;

    if(poll(&ufds, 1, 10) != 1) {
        fprintf(stderr, "No stdin data...\n");
        exit(-1);
    }

    while(getline(&line, &len, stdin) > 0) {
        file_buff = g_list_append(file_buff, g_strdup(line));
    }

    buff = store_file(to);

    fd_to = fopen_or_warn(to, WRITE);

    length = g_list_length(file_buff);

    for(i=0; i < length; i++) {
        line = g_list_nth_data(file_buff, i);
        if(line != NULL) {
            fprintf(fd_to, "%s", line);
            g_free(line);
        }
    }

    fclose(fd_to);

    write_buff(to, buff);

    return 0;
}

int called_with_line(char *to, int num)
{
    unsigned int len = 0;
    char *line       = NULL;
    FILE *fd_to      = NULL;
    GList *buff      = NULL;
    struct stat     st_stat;

    if(stat(to, &st_stat) != 0) {
        fprintf(stderr, "Fail to open [%s] !!\n", to);
        exit(1);
    }

    buff = store_file(to);

    write_buff_by_line(num, to,  BEGIN, buff);

    fd_to = fopen_or_warn(to, APPEND);

    while(getline(&line, &len, stdin) > 0) {
        fprintf(fd_to, "%s", line);
    }

    fclose(fd_to);

    write_buff_by_line(num, to, END, buff);

    return 0;
}

int main(int argc, char *argv[])
{
    int opt = 0;

    if(argc < 2) {
        return show_help(argv[0]);
    }

    while((opt = getopt(argc, argv, "vl:h")) != EOF) {
        if(opt == 'v') {
            return show_version();

        } else if (opt == 'l') {
            if(argc == 4) return called_with_line(argv[3], atoi(argv[2]));

        } else if ( (opt == 'h') || (opt == '?')) {
            return show_help(argv[0]);
        }
        printf("opt = [%c]\n", opt);
    }

    return called_by_pipe(argv[1]);
}
