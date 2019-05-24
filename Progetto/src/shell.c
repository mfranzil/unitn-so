#include "shell.h"

int ppid;
int stato = 1;                   /*Stato della centralina. */
int changed = 0;                 /* Modifiche alla message queue? */
int children_pids[MAX_CHILDREN]; /* array contenenti i PID dei figli */
int fd;
int max_index = 1;
int flag = 0;

int main(int argc, char *argv[]) {
    char(*buf)[MAX_BUF_SIZE] = malloc(MAX_BUF_SIZE * sizeof(char *)); /* array che conterrà i comandi da eseguire */
    char __out_buf[MAX_BUF_SIZE];                                     /* Per la stampa dell'output delle funzioni */
    char *name = get_shell_text();                                    /* Mostrato a ogni riga della shell */

    int cmd_n;        /* numero di comandi disponibili */
    int device_i = 0; /* indice progressivo dei dispositivi */

    char pipe[MAX_BUF_SIZE];

    int j;
    key_t key;
    int msgid;
    key_t key_sh;
    int msgid_sh;
    key_t key_shell;
    int msgid_shell;

    char current_msg[MAX_BUF_SIZE] = "0|";

    char tmp_c[MAX_BUF_SIZE];

    char child[8];

    int i;

    signal(SIGUSR1, stop_sig);
    signal(SIGTERM, cleanup_sig);
    signal(SIGUSR2, link_child);
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, handle_sig);

    /* Inizializzo l'array dei figli */
    for (j = 0; j < MAX_CHILDREN; j++) {
        children_pids[j] = -1; /* se è -1 non contiene nulla */
    }

    get_pipe_name((int)getpid(), pipe);
    mkfifo(pipe, 0666);
    fd = open(pipe, O_RDWR);

    /* PID del launcher. */
    ppid = atoi(argv[1]);

    /* Credo message queue tra shell e launcher */
    key = ftok("/tmp", 100000);
    msgid = msgget(key, 0666 | IPC_CREAT);
    message.mesg_type = 1;

    /*Creo message queue per comunicare shellpid */
    key_sh = ftok("/tmp", 200000);
    msgid_sh = msgget(key_sh, 0666 | IPC_CREAT);
    message.mesg_type = 1;

    key_shell = ftok("/tmp/ipc/shellqueue", 1);
    msgid_shell = msgget(key_shell, 0666 | IPC_CREAT);
    message.mesg_type = 1;

    sprintf(message.mesg_text, "%d", (int)getpid());
    msgsnd(msgid_sh, &message, MAX_BUF_SIZE, 0);
    
    /* Ready */
    system("clear");

    while (1) {
        if (stato) {
            /*Scrive numero devices e elenco dei pid a launcher. */
            if (changed) {
                /*Ripulisco Forzatamente. */
                msgrcv(msgid, &message, sizeof(message), 1, IPC_NOWAIT);

                sprintf(tmp_c, "%d|", device_i);
                i = 0;
                while (i < device_i) {
                    sprintf(child, "%d|", children_pids[i]);
                    strcat(tmp_c, child);
                    i++;
                }
                message.mesg_type = 1;
                sprintf(message.mesg_text, "%s", tmp_c);
                sprintf(current_msg, "%s", message.mesg_text);
                msgsnd(msgid, &message, sizeof(message), 0);
                changed = 0;
            } else {
                /*Ripulisco forzatamente. */
                msgrcv(msgid, &message, sizeof(message), 1, IPC_NOWAIT);
                message.mesg_type = 1;
                sprintf(message.mesg_text, "%s", current_msg);
                msgsnd(msgid, &message, sizeof(message), 0);
            }

            printf("\033[0;32m%s\033[0m:\033[0;31mCentralina\033[0m$ ", name);
            cmd_n = parse(buf, cmd_n);

            if (strcmp(buf[0], "help") == 0) { /* guida */
                printf(HELP_STRING);
            } else if (strcmp(buf[0], "list") == 0) {
                if (cmd_n != 0) {
                    printf(LIST_STRING);
                } else {
                    __list(children_pids);
                }
            } else if (strcmp(buf[0], "info") == 0) {
                if (cmd_n != 1) {
                    printf(INFO_STRING);
                } else {
                    __info(atoi(buf[1]), children_pids);
                }
            } else if (strcmp(buf[0], "switch") == 0) {
                if (cmd_n != 3) {
                    printf(SWITCH_STRING);
                } else if (strcmp(buf[2], "riempimento") == 0) {
                    printf("Operazione permessa solo manualmente.\n");
                } else {
                    __switch(atoi(buf[1]), buf[2], buf[3], children_pids);
                }
            } else if (strcmp(buf[0], "add") == 0) {
                if (cmd_n != 1) {
                    printf(ADD_STRING);
                } else {
                    changed = add_shell(buf, &device_i, children_pids, __out_buf);
                    printf("%s", __out_buf);
                    max_index++;
                }
            } else if (strcmp(buf[0], "del") == 0) {
                if (cmd_n != 1) {
                    printf(DEL_STRING);
                } else {
                    flag = 1;
                    __del(atoi(buf[1]), children_pids, __out_buf, flag);
                    printf("%s", __out_buf);
                }
            } else if (strcmp(buf[0], "link") == 0 && strcmp(buf[2], "to") == 0) {
                if (cmd_n != 3) {
                    printf(LINK_STRING);
                } else {
                    __link(atoi(buf[1]), atoi(buf[3]), children_pids);
                }
            } else if (strcmp(buf[0], "exit") == 0) { /* supponiamo che l'utente scriva solo "exit" per uscire */
                kill(ppid, SIGTERM);
                break;
            } else if (strcmp(buf[0], "\0") == 0) { /* a capo a vuoto */
                continue;
            } else { /*tutto il resto */
                printf(UNKNOWN_COMMAND);
            }
        } else {
            /*Ripulisco forzatamente. */
            msgrcv(msgid, &message, MAX_BUF_SIZE, 1, IPC_NOWAIT);
            /*A centralina spenta continuo a scrivere il vecchio stato sulla messagequeue. */
            sprintf(message.mesg_text, "%s", current_msg);
            msgsnd(msgid, &message, MAX_BUF_SIZE, 0);
            getchar(); /*Per ignorare i comandi quando non accessa. */
        }
    }
    free(buf);
    return 0;
}

int add_shell(char buf[][MAX_BUF_SIZE], int *device_i, int *children_pids, char *__out_buf) {
    if (strcmp(buf[1], "bulb") == 0 || strcmp(buf[1], "fridge") == 0 || strcmp(buf[1], "window") == 0 || strcmp(buf[1], "hub") == 0 || strcmp(buf[1], "timer") == 0) {
        (*device_i)++;
        if (__add(buf[1], *device_i, children_pids, __out_buf) == 0) {
            /* Non c'è spazio, ci rinuncio */
            (*device_i)--;
            return 0;
        } else {
            return 1;
        };
    } else {
        sprintf(__out_buf, "Dispositivo non ancora supportato\n");
        return 0;
    }
}

void cleanup_sig(int sig) {
    printf("Chiusura della centralina in corso...\n");
    int i = 0;
    for (i = 1; i < max_index; i++) {
        key_t key = ftok("/tmp/ipc/mqueues", i);
        int msgid = msgget(key, 0666 | IPC_CREAT);
        msgctl(msgid, IPC_RMID, NULL);
    }
    key_t key_shell = ftok("/tmp/ipc/shellqueue", 1);
    int msgid_shell = msgget(key_shell, 0666 | IPC_CREAT);
    message.mesg_type = 1;
    msgctl(msgid_shell, IPC_RMID, NULL);
    kill(ppid, SIGTERM);
    kill(0, SIGKILL);
}

void handle_sig(int sig) {
    kill(ppid, SIGTERM);
}

void stop_sig(int sig) {
    if (stato) {
        stato = 0;
        printf("La centralina è stata spenta. Nessun comando sarà accettato.\n");
    } else {
        stato = 1;
        system("clear");
        printf("\nLa centralina è accesa. Premi Invio per proseguire.\n");
    }
}

void link_child(int signal) {
<<<<<<< HEAD
    printf("ARRRIVATO SIGUSR2 FLAG: %d\n",flag);
    if(flag){
    int n_devices;
    int ret;
    int q, j;
    char n_dev_str[100];
    int __count;
    char tmp_buf[MAX_BUF_SIZE];
    char** vars;
    char** son_j;
=======
    if (flag) {
        int n_devices;
        int ret;
        int q, j;
        char n_dev_str[100];
        int __count;
        char tmp_buf[MAX_BUF_SIZE];
        char **vars;
        char **son_j;
>>>>>>> b6abd4485d6edac12a751501ade6164089bc2f0f

        key_t key_shell = ftok("/tmp/ipc/shellqueue", 1);
        int msgid_shell = msgget(key_shell, 0666 | IPC_CREAT);
        message.mesg_type = 1;

        ret = msgrcv(msgid_shell, &message, sizeof(message), 1, IPC_NOWAIT);
        if (ret != -1) {
            q = 0;
            while (!(message.mesg_text[q] == '-')) {
                n_dev_str[q] = message.mesg_text[q];
                q++;
            }
            n_dev_str[q] = '\0';
            n_devices = atoi(n_dev_str);
            if (n_devices > 0) {
                __count = n_devices;
                sprintf(tmp_buf, "%s", message.mesg_text);
                vars = NULL;
                vars = split_sons(tmp_buf, __count);
                j = 0;
                while (j <= __count) {
                    if (j >= 1) {
                        printf("\nVars %d: %s\n", j, vars[j]);
                        son_j = split(vars[j]);
                        __add_ex(son_j, children_pids);
                        printf("\nADD_EX GOOD\n");
                    }
                    j++;
                }
            }
        }
        flag = 0;
    }
}
