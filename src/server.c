#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define MAX_CLIENTS 40
#define SA struct sockaddr

static _Atomic unsigned int cli_count = 0;
static long long int uid = 10;
int adminactive = 0;
int admin_private = 0;
long int array_size = 524280; // 64 Kilobytes

typedef struct {
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[32], rno[32];
    int admin;
    int uni;
    int private_uid;
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER,
    adminclient_mutex = PTHREAD_MUTEX_INITIALIZER,
    fileSending_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length) {
    for (int i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void print_ip_addr(struct sockaddr_in addr) {
    printf("\n %d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void queue_add(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *s, int uid) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->uid != uid) {
                if (write(clients[i]->sockfd, s, strlen(s)) < 0) {
                    // Handle write error
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

int checkStudent(char *s, int uid) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->uid != uid) {
                if (strcmp(clients[i]->rno, s) == 0 || strcmp(clients[i]->name, s) == 0) {
                    printf("[+] present\n");
                    pthread_mutex_unlock(&clients_mutex);
                    return i + 1;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("[+] absent");
    return 0;
}

void sendFileToNonAdmin(char *fileName, int i) {
    pthread_mutex_lock(&fileSending_mutex);
    {
        char nc, filearray[array_size];
        bzero(filearray, sizeof(filearray));
        int pointer;
        pointer = 0;

        printf("[+] Entered sendFileToNonAdmin()\n%s\n%d\n", fileName, i);
        send(clients[i]->sockfd, "$[+]$incoming$file$from$admi$", strlen("$[+]$incoming$file$from$admi$"), 0);
        send(clients[i]->sockfd, fileName, sizeof(fileName), 0);

        printf("\n\nCode:\n");

        FILE *fp;
        fp = fopen(fileName, "r");

        printf("\n\nFile content:\n");
        while (fscanf(fp, "%c", &nc) != EOF) {
            filearray[pointer] = nc;
            pointer++;
        }
        fclose(fp);
        printf("%s\n", filearray);

        send(clients[i]->sockfd, filearray, sizeof(filearray), 0);
    }
    pthread_mutex_unlock(&fileSending_mutex);
}

char *nameOfAdmin() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->admin == 1) {
                return clients[i]->name;
            }
        }
    }

    return "Admin";
}

void *handle_client(void *arg) {
    char buff_out[5000], buff_out2[5000], buff_name[50];
    char name[32], rno[32];
    int leave_flag = 0;
    cli_count++;

    client_t *cli = (client_t *)arg;

    //name
    if (recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) < 1 || strlen(name) >= 32 - 1) {
        printf("\nEnter the name correctly...\n");
        leave_flag = 1;
    }
    if (recv(cli->sockfd, rno, 32, 0) <= 0 || strlen(rno) < 1 || strlen(rno) >= 32 - 1) {
        printf("\nEnter the roll number correctly...%s\n", rno);
        leave_flag = 1;
    }
    else {
        cli->admin = 0;
        strcpy(cli->name, name);
        strcpy(cli->rno, rno);
        sprintf(buff_out, "\n%s_%s has joined\n", cli->name, cli->rno);
        printf("%s", buff_out);
        send_message(buff_out, cli->uid);
    }
    bzero(buff_out, 5000);

    while (1) {
        if (leave_flag) {
            break;
        }
        int receive = recv(cli->sockfd, buff_out, 5000, 0);
        if (receive > 0) {
            if (strlen(buff_out) > 0) {
                str_trim_lf(buff_out, strlen(buff_out));
                if (strcmp(buff_out, "set-admin") == 0) {
                    if (adminactive == 0) {
                        sprintf(buff_out, "[+] Enter Password to become admin: ");
                        write(cli->sockfd, buff_out, strlen(buff_out));
                        recv(cli->sockfd, buff_out, 5000, 0);
                        sscanf(buff_out, "%s : %s\n", buff_out2, buff_out);

                        if (strcmp(buff_out, "vit@2023") == 0) {
                            pthread_mutex_lock(&adminclient_mutex);
                            sprintf(buff_out, "[+] You are now admin");
                            write(cli->sockfd, buff_out, strlen(buff_out));
                            sprintf(buff_out, "[+] %s has been set as admin", cli->name);
                            send_message(buff_out, cli->uid);
                            printf("%s\n", buff_out);
                            adminactive = 1;
                            cli->admin = 1;
                            pthread_mutex_unlock(&adminclient_mutex);
                        }
                        else {
                            sprintf(buff_out, ".\n[+] Wrong password entered...\n.");
                            write(cli->sockfd, buff_out, strlen(buff_out));
                        }
                    }
                    else if (adminactive == 1) {
                        if (!cli->admin) {
                            sprintf(buff_out, "[+] There is already an Admin present inside this server\nNo more admins can be created.");
                            write(cli->sockfd, buff_out, strlen(buff_out));
                            printf("[+] %s requested to become an admin\n", cli->name);
                        }
                        else {
                            sprintf(buff_out, "[+] You are already an admin...");
                            write(cli->sockfd, buff_out, strlen(buff_out));
                        }
                    }
                }
                else if (strcmp(buff_out, "logout") == 0) {
                    if (adminactive == 1) {
                        if (cli->admin) {
                            pthread_mutex_lock(&adminclient_mutex);
                            sprintf(buff_out, "[+] You are not an admin anymore");
                            write(cli->sockfd, buff_out, strlen(buff_out));
                            sprintf(buff_out, "[+] %s has been removed as admin", cli->name);
                            send_message(buff_out, cli->uid);
                            printf("%s\n", buff_out);
                            adminactive = 0;
                            cli->admin = 0;
                            pthread_mutex_unlock(&adminclient_mutex);
                        }
                        else {
                            sprintf(buff_out, "[+] You are not an admin...");
                            write(cli->sockfd, buff_out, strlen(buff_out));
                        }
                    }
                    else if (adminactive == 0) {
                        sprintf(buff_out, "[+] You are already not an admin");
                        write(cli->sockfd, buff_out, strlen(buff_out));
                    }
                }
                else {
                    bzero(buff_name, sizeof(buff_name));
                    bzero(buff_out2, 5000);
                    sscanf(buff_out, "%s : %s\n", buff_name, buff_out2);
                    if (strcmp(buff_out2, "send") == 0) {
                        if (!cli->admin) {
                            char fileName[50];
                            printf("[+] send command received from %s_%s\n", cli->name, cli->rno);
                            bzero(buff_out2, sizeof(buff_out2));
                            sprintf(buff_out2, "[+] Enter file name: ");
                            write(cli->sockfd, buff_out2, strlen(buff_out2));
                            recv(cli->sockfd, fileName, 50, 0);
                            printf("[+] file name -> %s\n", fileName);

                            // Receiving program file from non-admin user to admin (i.e., to server)...
                            {
                                char location[5000], directory[1000];
                                bzero(directory, sizeof(directory));

                                system("mkdir ServerFiles");

                                sprintf(directory, "mkdir ServerFiles/%s", nameOfAdmin());
                                system(directory);

                                bzero(directory, sizeof(directory));

                                sprintf(directory, "mkdir ServerFiles/%s/%s", nameOfAdmin(), cli->rno);
                                system(directory);

                                sprintf(location, "ServerFiles/%s/%s/%s", nameOfAdmin(), cli->rno, fileName);
                                char filearray[array_size];

                                FILE *fp;
                                fp = fopen(location, "w");
                                printf("[+] new program file created...(location: %s)", location);

                                bzero(filearray, sizeof(filearray));

                                recv(cli->sockfd, filearray, sizeof(filearray), 0);

                                fprintf(fp, filearray);
                                fclose(fp);

                                printf("[+] received code:\n%s\n", filearray);
                            }
                        }
                        if (cli->admin) {
                            printf("[+] send command received...from %s_%s\n", cli->name, cli->rno);
                            bzero(buff_out2, sizeof(buff_out2));
                            sprintf(buff_out2, "[+] Enter Student name or roll number: ");
                            write(cli->sockfd, buff_out2, strlen(buff_out2));
                            bzero(buff_out2, sizeof(buff_out2));
                            bzero(buff_out, 5000);
                            recv(cli->sockfd, buff_out, 5000, 0); // Student roll or name received

                            int isStudent = checkStudent(buff_out, cli->uid); // 0-absent or any number - present

                            if (isStudent != 0) {
                                bzero(buff_out, 5000);
                                sprintf(buff_out, "[+] Student is present.");
                                send(cli->sockfd, buff_out, 5000, 0);
                                sprintf(buff_out2, "[+] Enter file name: ");
                                write(cli->sockfd, buff_out2, strlen(buff_out2));
                                bzero(buff_out2, 5000);
                                recv(cli->sockfd, buff_out2, 5000, 0);

                                // Sending file to non-admin user...
                                sendFileToNonAdmin(buff_out2, isStudent - 1);
                            } else {
                                bzero(buff_out, 5000);
                                sprintf(buff_out, "[+] Student is absent.");
                                send(cli->sockfd, buff_out, 5000, 0);
                            }
                        }
                    } else {
                        send_message(buff_out, cli->uid);
                    }
                }
            }
        }
        else if (receive == 0 || strcmp(buff_out, "exit") == 0) {
            if (adminactive == 1 && cli->admin == 1) {
                adminactive = 0;
                sprintf(buff_out, "The Admin (%s_%s) has left", cli->name, cli->rno);
                send_message(buff_out, cli->uid);
                printf("%s\n", buff_out);
                leave_flag = 1;
                break;
            }
            else {
                sprintf(buff_out, "\n%s_%s has left\n", cli->name, cli->rno);
                printf("%s", buff_out);
                send_message(buff_out, cli->uid);
                leave_flag = 1;
                break;
            }
        }
        else {
            printf("\nERROR: -1\n");
            leave_flag = 1;
        }
        bzero(buff_out, 5000);
    }
    close(cli->sockfd);
    queue_remove(cli->uid);
    free(cli);
    cli_count--;
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char **argv) {
    if (argc == 3 || argc == 2 || argc == 1) {;}
    else {
        printf("\nInvalid number of arguments passed...\n(hint: syntax is like '%s <port_no> <ip_address>')\n", argv[0]);
        exit(0);
    }

    int sockfd, newsockfd;
    socklen_t n;
    struct sockaddr_in servaddr, clientaddr;
    int option = 1;

    pthread_t tid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("\nERROR: socket()\n");
        exit(0);
    }

    // Socket settings
    servaddr.sin_family = AF_INET;
    if (argc == 2 || argc == 3)
        servaddr.sin_port = htons(atoi(argv[1]));
    else
        servaddr.sin_port = htons(8080);
    if (argc == 3)
        servaddr.sin_addr.s_addr = inet_addr(argv[2]);
    else
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Signals
    signal(SIGPIPE, SIG_IGN);

    if (setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char *)&option, sizeof(option)) < 0) {
        printf("\nError: setsockopt\n");
        return EXIT_FAILURE;
    }

    // Bind
    if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        if (argc == 3) {
            servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            if (bind(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
                printf("\nFailed to bind the socket with local IP too...(The given port might be already in use... try with a different port...)\n");
                exit(0);
            } else
                printf("\nSuccessfully Binded...\n");
        } else {
            printf("\nFailed to bind the socket...\n");
            exit(0);
        }
    }

    // Listen
    if (listen(sockfd, 5) < 0) {
        printf("\nERROR: listen\n");
        exit(0);
    }

    system("cls || clear");
    printf("\n==== VIT SERVER ====\n");

    while (1) {
        n = sizeof(clientaddr);
        newsockfd = accept(sockfd, (SA *)&clientaddr, &n);

        // Check for max clients
        if ((cli_count + 1) == MAX_CLIENTS) {
            printf("\nMaximum clients connected\nConnection rejected ...\n");
            print_ip_addr(clientaddr);
            printf(":%d\n", clientaddr.sin_port);
            close(newsockfd);
            continue;
        }

        // Client settings
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = clientaddr;
        cli->sockfd = newsockfd;
        cli->uid = uid++;

        // Add client to queue
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void *)cli);

        // Reduce CPU usage
        sleep(1);
    }
    return EXIT_SUCCESS;
}
