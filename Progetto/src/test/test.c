#include <string.h>
#include "../util.h"
/*
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

void hub_tree_print(char **vars) {
    if (strcmp(vars[0], HUB_S) == 0) {
        printf("Hub (PID: %s, Indice: %s), Stato: %s, Collegati: %s",
               vars[1], vars[2], atoi(vars[3]) ? "Acceso" : "Spento", vars[4]);
    } else {
        char device_name[MAX_BUF_SIZE];
        get_device_name(atoi(vars[0]), device_name);
        device_name[0] += 'A' - 'a';

        printf("%s, (PID %s, Indice %s)", device_name, vars[1], vars[2]);
    }
}

void hub_tree_parser(char *__buf, int __count) {
    char *tokenizer = strtok(__buf, "|");
    char *old = NULL;
    int level = 0;

    //printf("Level %d\n", level);

    char **vars = malloc((10) * sizeof(*vars));
    int i = 0;
    int to_be_printed = 1;

    while (tokenizer != NULL) {
        int j;
        if (strcmp(tokenizer, "<!") == 0) {
            to_be_printed += atoi(old) - 1;
            if (level > 0) {
                printf("\n");
                for (int j = 0; j < level; j++) {
                    printf("  ");
                }
                printf("∟ ");
            }
            i = 0;
            ++level;  //   printf("\nLevel %d\n", ++level);
            hub_tree_print(vars);
        } else if (strcmp(tokenizer, "!>") == 0) {
            --level;  //   printf("\nLevel %d\n", --level);
            if (strcmp(old, "<!") == 0 && to_be_printed > 0) {
                i = 0;
                printf("\n");
                for (int j = 0; j < level; j++) {
                    printf("  ");
                }
                printf("∟ ");
                hub_tree_print(vars);
                to_be_printed--;
            }
        } else if (strcmp(tokenizer, "!") == 0) {
            if (to_be_printed > 0) {
                i = 0;
                printf("\n");
                for (int j = 0; j < level; j++) {
                    printf("  ");
                }
                printf("∟ ");
                hub_tree_print(vars);
                to_be_printed--;
            }
        } else {
            vars[i++] = tokenizer;
            //printf("%s, ", tokenizer);
        }
        old = tokenizer;
        tokenizer = strtok(NULL, "|");
    }
}

char **Nsplit(char *__buf) {
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
            __count = 5;
            break;
        default:
            __count = 1;
            break;
    }

    return hub_tree_parser(__buf, __count);
}
*/
int main() {
    mkfifo("/tmp/ipc/12388", 0666);
    char buffer[MAX_BUF_SIZE];
    sprintf(buffer, "1|12388|1|0|0");
    int fd = open("/tmp/ipc/12388", O_RDWR);
    write(fd, buffer, MAX_BUF_SIZE);
    printf(buffer);
    close(fd);
    
/*
    while (1) {
        printf("\n> ");
        scanf("%s", buf);
        Nsplit(buf);
    }*/
}