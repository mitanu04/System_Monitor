#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

volatile sig_atomic_t got_signal = 0;
volatile sig_atomic_t stop_signal = 0;

typedef struct {
    int treasureID;
    char userName[100];
    float latitude;
    float longitude;
    char clueText[100];
    int value;
} treasureInfo;

void handler(int sig) {
    if (sig == SIGUSR1) {
        got_signal = 1;
        printf("[monitor] Received SIGUSR1\n");
        fflush(stdout);
    } else if (sig == SIGUSR2) {
        stop_signal = 1;
        printf("[monitor] Received SIGUSR2\n");
        fflush(stdout);
    }
}

void setup_signals() {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
}

void handle_list_hunts() {
  write(STDOUT_FILENO, "[monitor] Executing list_hunts\n", 31);

  DIR* huntsDir = opendir("Hunts");
    if (!huntsDir) {
        perror("Could not open Hunts directory");
        return;
    }

    struct dirent* huntEntry;

    while ((huntEntry = readdir(huntsDir)) != NULL) {
        if (strcmp(huntEntry->d_name, ".") == 0 || strcmp(huntEntry->d_name, "..") == 0)
            continue;

        char huntPath[512];
        snprintf(huntPath, sizeof(huntPath), "Hunts/%s", huntEntry->d_name);

        DIR* huntSubDir = opendir(huntPath);
        if (!huntSubDir) continue;

        struct dirent* fileEntry;
        int treasureCount = 0;

        while ((fileEntry = readdir(huntSubDir)) != NULL) {
            if (strstr(fileEntry->d_name, ".dat")) {
	      char filePath[1024];
                snprintf(filePath, sizeof(filePath), "%s/%s", huntPath, fileEntry->d_name);
                int fd = open(filePath, O_RDONLY);
                if (fd == -1) continue;

                treasureInfo t;
                ssize_t bytes;
                while ((bytes = read(fd, &t, sizeof(treasureInfo))) == sizeof(treasureInfo)) {
                    treasureCount++;
                }
                close(fd);
            }
        }

        closedir(huntSubDir);
        printf("Hunt %s: %d treasures\n", huntEntry->d_name, treasureCount);
    }

    closedir(huntsDir);
}

void handle_list_treasures(const char* huntID) {
    char dirPath[256];
    snprintf(dirPath, sizeof(dirPath), "Hunts/%s", huntID);

    DIR* dir = opendir(dirPath);
    if (!dir) {
        perror("Could not open hunt directory");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".dat")) {
            char filePath[512];
            snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);
            int fd = open(filePath, O_RDONLY);
            if (fd == -1) continue;

            treasureInfo t;
            while (read(fd, &t, sizeof(treasureInfo)) == sizeof(treasureInfo)) {
                printf("ID: %d | User: %s | Lat: %.2f | Long: %.2f | Value: %d\nClue: %s\n\n",
                    t.treasureID, t.userName, t.latitude, t.longitude, t.value, t.clueText);
            }

            close(fd);
        }
    }

    closedir(dir);
}

void handle_view_treasure(const char* huntID, const char* treasureID) {
    char dirPath[256];
    snprintf(dirPath, sizeof(dirPath), "Hunts/%s", huntID);

    DIR* dir = opendir(dirPath);
    if (!dir) {
        perror("Could not open hunt directory");
        return;
    }

    int found = 0;
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".dat")) {
            char filePath[512];
            snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);
            int fd = open(filePath, O_RDONLY);
            if (fd == -1) continue;

            treasureInfo t;
            while (read(fd, &t, sizeof(treasureInfo)) == sizeof(treasureInfo)) {
                if (t.treasureID == atoi(treasureID)) {
                    printf("ID: %d | User: %s | Lat: %.2f | Long: %.2f | Value: %d\nClue: %s\n\n",
                        t.treasureID, t.userName, t.latitude, t.longitude, t.value, t.clueText);
                    found = 1;
                    break;
                }
            }

            close(fd);
            if (found) break;
        }
    }

    closedir(dir);

    if (!found) {
        printf("Treasure with ID %s not found in hunt %s\n", treasureID, huntID);
    }
}

void process_command(const char* cmd_line) {
    char command[256], huntID[100], treasureID[100];
    memset(huntID, 0, sizeof(huntID));
    memset(treasureID, 0, sizeof(treasureID));

    if (sscanf(cmd_line, "%s %s %s", command, huntID, treasureID) >= 1) {
        if (strcmp(command, "list_hunts") == 0) {
            handle_list_hunts();
        }
        else if (strcmp(command, "list_treasures") == 0 && strlen(huntID) > 0) {
            handle_list_treasures(huntID);
        }
        else if (strcmp(command, "view_treasure") == 0 && strlen(huntID) > 0 && strlen(treasureID) > 0) {
            handle_view_treasure(huntID, treasureID);
        }
        else if (strcmp(command, "stop") == 0) {
            printf("Stopping monitor...\n");
            usleep(3000000);  // simulate delay
            exit(0);
        }
        else {
            printf("Invalid command or missing parameters.\n");
        }
    }
}

int main() {
    setup_signals();
    printf("Monitor running with PID %d\n", getpid());
    fflush(stdout);
    
    while (1) {
        pause();  // așteaptă semnal

        if (got_signal) {
            got_signal = 0;

            int fd = open("command.txt", O_RDONLY);
            if (fd < 0) continue;

            char cmd[256] = {0};
            read(fd, cmd, sizeof(cmd) - 1);
            close(fd);


            if (strncmp(cmd, "list_hunts", 10) == 0) {
                handle_list_hunts();
            } else if (strncmp(cmd, "list_treasures", 14) == 0) {
                char huntID[100];
                sscanf(cmd, "list_treasures %s", huntID);
                handle_list_treasures(huntID);
            } else if (strncmp(cmd, "view_treasure", 13) == 0) {
                char huntID[100], treasureID[100];
                sscanf(cmd, "view_treasure %s %s", huntID, treasureID);
                handle_view_treasure(huntID, treasureID);
            } else if (strncmp(cmd, "stop", 4) == 0) {
                break;
            }
        }

        if (stop_signal) break;
    }

    usleep(500000);  // simulare întârziere la închidere
    return 0;
}
