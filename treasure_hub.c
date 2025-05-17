/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

pid_t monitor_pid = -1;
int monitor_running = 0;
int monitor_exited = 0;

void sigchld_handler(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0);
    printf("Monitor exited with status %d\n", WEXITSTATUS(status));
    monitor_running = 0;
    monitor_exited = 1;
}

void start_monitor() {
    if (monitor_running) {
        printf("Monitor already running.\n");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid < 0) {
        perror("fork failed");
        return;
    }

    if (monitor_pid == 0) {
        execl("./monitor", "monitor", NULL);
        perror("execl failed");
        exit(1);
    } else {
        printf("Monitor started with PID %d\n", monitor_pid);
        monitor_running = 1;
        monitor_exited = 0;
 usleep(150000);
    
    }
}


void send_command(const char* command) {
    if (!monitor_running) {
        printf("Monitor is not running. Command rejected.\n");
        return;
    }

    int fd = open("command.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        return;
    }

    dprintf(fd, "%s\n", command);
    close(fd);
    usleep(100000);
    kill(monitor_pid, SIGUSR1);
}

void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor is not running.\n");
        return;
    }

    int fd = open("command.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        return;
    }

    dprintf(fd, "stop\n");
    close(fd);
    kill(monitor_pid, SIGUSR2);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char command[256];
    while (1) {
        printf("hub> ");
        fflush(stdout);

        ssize_t len = read(STDIN_FILENO, command, sizeof(command) - 1);
        if (len <= 0) break;
        command[len] = '\0';
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        }
    
    else if (strncmp(command, "list_hunts", 10) == 0 ||
           strncmp(command, "list_treasures", 14) == 0 ||
           strncmp(command, "view_treasure", 13) == 0) {
    if (!monitor_running) {
        printf("Monitor is not running. Command rejected.\n");
    } else if (monitor_exited) {
        printf("Monitor is shutting down. Wait until it exits.\n");
    } else {
        send_command(command);
        printf("hub> ");      // ← promptul revine forțat aici
        fflush(stdout);
    }
    }
	else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(command, "exit") == 0) {
            if (monitor_running) {
                printf("Monitor still running. Use stop_monitor first.\n");
            } else {
                break;
            }
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

pid_t monitor_pid = -1;
int monitor_running = 0;
int monitor_exited = 0;

void sigchld_handler(int sig) {
    int status;
    waitpid(monitor_pid, &status, 0);
    printf("Monitor exited with status %d\n", WEXITSTATUS(status));
    monitor_running = 0;
    monitor_exited = 1;
}

void start_monitor() {
    if (monitor_running) {
        printf("Monitor already running.\n");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid < 0) {
        perror("fork failed");
        return;
    }

    if (monitor_pid == 0) {
        execl("./monitor", "monitor", NULL);
        perror("execl failed");
        exit(1);
    } else {
        printf("Monitor started with PID %d\n", monitor_pid);
        monitor_running = 1;
        monitor_exited = 0;
        usleep(150000); // short delay to allow monitor to print its output
    }
}

void send_command(const char* command) {
    if (!monitor_running) {
        printf("Monitor is not running. Command rejected.\n");
        return;
    }

    int fd = open("command.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        return;
    }

    dprintf(fd, "%s\n", command);
    close(fd);
    usleep(100000);
    kill(monitor_pid, SIGUSR1);
}

void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor is not running.\n");
        return;
    }

    int fd = open("command.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        return;
    }

    dprintf(fd, "stop\n");
    close(fd);
    kill(monitor_pid, SIGUSR2);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char command[256];
    while (1) {
        printf("hub> ");
        fflush(stdout);

        ssize_t len = read(STDIN_FILENO, command, sizeof(command) - 1);
        if (len <= 0) break;
        command[len] = '\0';
        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        } else if (strncmp(command, "list_hunts", 10) == 0 ||
                   strncmp(command, "list_treasures", 14) == 0 ||
                   strncmp(command, "view_treasure", 13) == 0) {
            if (!monitor_running) {
                printf("Monitor is not running. Command rejected.\n");
            } else if (monitor_exited) {
                printf("Monitor is shutting down. Wait until it exits.\n");
            } else {
                send_command(command);
		usleep(150000);         
     
        fflush(stdout);
		
            }
        } else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(command, "exit") == 0) {
            if (monitor_running) {
                printf("Monitor still running. Use stop_monitor first.\n");
            } else {
                break;
            }
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
