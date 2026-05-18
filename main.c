#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>      
#include <signal.h>   

#define MAX_WORD 400
#define VAL 30
#define PORT 8080
#define MAX_CLIENTS 10 

void run_path(char* input);
void start_network_server();
void sigchld_handler(int signo);
void exec_single_command(char* cmd_part);

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction failed");
        exit(1);
    }
    start_network_server();
    return 0;
}
void sigchld_handler(int signo) {
    (void)signo; 
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
void exec_single_command(char* cmd_part) {
    char* str = NULL;
    char* token = NULL;
    char command[MAX_WORD], full_path[MAX_WORD];
    char* arg[VAL];
    int i = 0;

    arg[i] = strtok(cmd_part, " ");
    while (arg[i] != NULL) {
        i++;
        arg[i] = strtok(NULL, " ");
    }
    arg[i] = NULL;

    if (arg[0] == NULL) {
        exit(0);
    }
    if (strchr(arg[0], '/') != NULL) {
        execv(arg[0], arg);
        perror("execv failed");
        exit(6);
    }
    str = getenv("PATH");
    if (str == NULL) {
        printf("error finding PATH\n");
        exit(4);
    }
    strcpy(command, str);

    token = strtok(command, ":");
    while (token != NULL) {
        strcpy(full_path, token);
        strcat(full_path, "/");
        strcat(full_path, arg[0]);
        execv(full_path, arg); 
        token = strtok(NULL, ":");
    }
    printf("command not found: %s\n", arg[0]);
    exit(1);
}
void run_path(char* input) {
    char* pipe_pos = strchr(input, '|');
    char* left_cmd = NULL;
    char* right_cmd = NULL;
    int pipe_fds[2];
    pid_t pid1, pid2; 

    if (pipe_pos == NULL) {
        exec_single_command(input);
        return;
    }
    * pipe_pos = '\0'; 
    left_cmd = input;          
    right_cmd = pipe_pos + 1; 
    
    if (pipe(pipe_fds) == -1) {
        perror("pipe failed");
        exit(1);
    }
    if ((pid1 = fork()) == 0) {
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        exec_single_command(left_cmd);
        exit(1);
    }
    if ((pid2 = fork()) == 0) {
        dup2(pipe_fds[0], STDIN_FILENO);
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        exec_single_command(right_cmd);
        exit(1);
    }
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    exit(0);
}

void start_network_server() {
    struct sockaddr_in my_addr;
    int sockfd;
    int opt = 1, pid;
    char input[MAX_WORD];
    int bytes_read;
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1; 

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(2);
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) {
        perror("Error in binding");
        exit(3);
    }
    if (listen(sockfd, 3) < 0) {
        perror("listen failed");
        exit(7);
    }
    printf("Server listening on port %d with poll()...\n", PORT);
    memset(fds, 0, sizeof(fds));
    fds[0].fd = sockfd;
    fds[0].events = POLLIN; 
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
    }

    while (1) {
        int poll_count = poll(fds, nfds, -1);
        if (poll_count < 0) {
            perror("poll failed");
            continue;
        }
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int client_fd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len);
            if (client_fd < 0) {
                perror("accept failed");
            } else {
                int added = 0;
                for (int i = 1; i <= MAX_CLIENTS; i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = client_fd;
                        fds[i].events = POLLIN; 
                        if (i >= nfds) {
                            nfds = i + 1;
                        }
                        printf("Client %d connected from %s!\n", client_fd, inet_ntoa(client_addr.sin_addr));
                        dprintf(client_fd, "myshell@remote:~$ "); 
                        added = 1;
                        break;
                    }
                }
                if (!added) {
                    dprintf(client_fd, "Server full. Try again later.\n");
                    close(client_fd);
                }
            }
        }
        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) continue;

            if (fds[i].revents & POLLIN) {
                int current_client = fds[i].fd;
                memset(input, 0, MAX_WORD);
                bytes_read = recv(current_client, input, MAX_WORD - 1, 0);
                if (bytes_read <= 0) {
                    printf("Client %d disconnected.\n", current_client);
                    close(current_client);
                    fds[i].fd = -1; 
                    continue;
                }
                while (bytes_read > 0 && (input[bytes_read - 1] == '\n' || input[bytes_read - 1] == '\r')) {
                    input[bytes_read - 1] = '\0';
                    bytes_read--;
                }
                if (strlen(input) == 0) {
                    dprintf(current_client, "myshell@remote:~$ ");
                    continue;
                }
                if (strcmp(input, "leave") == 0) {
                    dprintf(current_client, "Goodbye!\n");
                    close(current_client);
                    fds[i].fd = -1;
                    continue;
                }
                if ((pid = fork()) == 0) {
                    dup2(current_client, STDIN_FILENO);
                    dup2(current_client, STDOUT_FILENO);
                    dup2(current_client, STDERR_FILENO);
                    for (int j = 0; j < nfds; j++) {
                        if (fds[j].fd != -1) close(fds[j].fd);
                    }
                    run_path(input);
                    exit(1);
                } 
                else if (pid > 0) {
                    dprintf(current_client, "myshell@remote:~$ ");
                }
            }
        }
    }
    close(sockfd);
}
