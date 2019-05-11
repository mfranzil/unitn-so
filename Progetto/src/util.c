#include "util.h"

int print_mode = 1;

void cprintf(const char *__restrict__ __format, ...) {
    if (print_mode) {
        va_list(args);
        //  cprintf("%d - ", time(NULL));
        va_start(args, __format);
        vprintf(__format, args);
    }
}

int parse(char buf[][MAX_BUF_SIZE], int cmd_n) {
    char ch;   // carattere usato per la lettura dei comandi
    int ch_i;  // indice del carattere corrente

    ch = ' ';
    ch_i = -1;
    cmd_n = 0;
    buf[cmd_n][0] = '\0';
    while (ch != EOF && ch != '\n') {
        ch = getchar();
        if (ch == ' ') {
            buf[cmd_n++][++ch_i] = '\0';
            ch_i = -1;
        } else {
            buf[cmd_n][++ch_i] = ch;
        }
    }
    buf[cmd_n][ch_i] = '\0';
    buf[cmd_n + 1][0] = '\0';

    return cmd_n;
}

char **split(char *__buf) {
    // Divide una stringa presa dalla pipe
    // a seconda del dispositivo.

    // La prima cifra di __buf è sempre il tipo di dispositivo.

    int device = __buf[0] - '0';
    int __count;

    switch (device) {
        case BULB:
            __count = BULB_PARAMETERS;
            break;
        case FRIDGE:
            __count = FRIDGE_PARAMETERS;
            break;
        case WINDOW:
            __count = WINDOW_PARAMETERS;
            break;
        case HUB:
            __count = HUB_PARAMETERS;
            break;
        default:
            __count = 1;
            break;
    }

    return split_fixed(__buf, __count);
}

char **split_fixed(char *__buf, int __count) {
    char *tokenizer = strtok(__buf, "|");
    char **vars = malloc(__count * sizeof(char *));
    int j = 0;

    while (tokenizer != NULL && j <= __count) {
        vars[j++] = tokenizer;
        tokenizer = strtok(NULL, "|");
    }

    return vars;
}

char *get_shell_text() {
    int uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        char hostname[MAX_BUF_SIZE];
        hostname[MAX_BUF_SIZE - 1] = '\0';
        gethostname(hostname, MAX_BUF_SIZE - 1);
        return strcat(strcat(pw->pw_name, "@"), hostname);
    }
    return "host";
}

void get_pipe_name(int pid, char *pipe_str) {
    sprintf(pipe_str, "%s%i", PIPES_POSITIONS, pid);
}

void get_device_name(int device_type, char *buf) {
    switch (device_type) {
        case BULB:
            sprintf(buf, "lampadina");
            break;
        case FRIDGE:
            sprintf(buf, "frigo");
            break;
        case WINDOW:
            sprintf(buf, "finestra");
            break;
        case CONTROLLER:
            sprintf(buf, "centralina");
            break;
        case HUB:
            sprintf(buf, "hub");
            break;
        default:
            sprintf(buf, "-");
            break;
    }
}

void get_device_name_str(char *device_type, char *buf) {
    if (strcmp(device_type, "bulb") == 0) {
        sprintf(buf, "lampadina");
    } else if (strcmp(device_type, "fridge") == 0) {
        sprintf(buf, "frigo");
    } else if (strcmp(device_type, "window") == 0) {
        sprintf(buf, "finestra");
    } else if (strcmp(device_type, "controller") == 0) {
        sprintf(buf, "centralina");
    } else if (strcmp(device_type, "hub") == 0) {
        sprintf(buf, "hub");
    } else {
        sprintf(buf, "-");
    }
}

int get_device_pid(int device_identifier, int *children_pids) {
    // prende come input l'indice/nome del dispositivo, ritorna il PID
    int i;
    for (i = 0; i < MAX_CHILDREN; i++) {  // l'indice i è logicamente indipendente dal nome/indice del dispositivo
        int children_pid = children_pids[i];
        if (children_pid != -1) {
            char **vars = NULL;
            get_device_info(children_pid, vars);
            fflush(stdout);
           // printf("%s, %s, %s", vars[0], vars[1], vars[2]);
            // I primi 3 parametri sono sempre tipo, pid, indice
            if (vars != NULL && atoi(vars[2]) == device_identifier) {
                free(vars);
                return children_pid;
            }
        }
    }
    return -1;
}

void get_device_info(int pid, char **vars) {
    char tmp[MAX_BUF_SIZE];

    kill(pid, SIGUSR1);

    char pipe_str[MAX_BUF_SIZE];
    get_pipe_name(pid, pipe_str);

    int fd = open(pipe_str, O_RDONLY);

    if (fd > 0) {
        read(fd, tmp, MAX_BUF_SIZE);
        vars = split(tmp);
        printf("%s, %s, %s; ", vars[0], vars[1], vars[2]);

        // Pulizia
        close(fd);
    }
}