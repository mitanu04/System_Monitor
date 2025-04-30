#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>  
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#define MAX_USER_LENGHT 100
#define MAX_PATH_SIZE 100
#define MAX_FILEPATH_SIZE 400
#define LINE_SIZE 256
#define MAX_BUFFER_SIZE 4096

typedef struct {
    int treasureID;
    char userName[MAX_USER_LENGHT];
    //gps coordinates
    float latitude;
    float longitude;

    char clueText[MAX_USER_LENGHT];
    int value;
} treasureInfo;


int is_file_empty(const char *huntID) {
    struct stat st;
    if (stat(huntID, &st) == -1) {
        perror("stat failed");
        return -1;  
    }
    return (st.st_size == 0);  
}


void inputTreasureInfo(treasureInfo* newTreasure) {
    printf("Enter Treasure ID: ");
    if (scanf("%d", &newTreasure->treasureID) != 1) {
        printf("Invalid input for Treasure ID.\n");
        return;
    }

    printf("Enter Username: ");
    if (scanf("%s", newTreasure->userName) != 1) { 
        printf("Invalid input for Username.\n");
        return;
    }

    printf("Enter Latitude: ");
    if (scanf("%f", &newTreasure->latitude) != 1) {
        printf("Invalid input for Latitude.\n");
        return;
    }

    printf("Enter Longitude: ");
    if (scanf("%f", &newTreasure->longitude) != 1) {
        printf("Invalid input for Longitude.\n");
        return;
    }

    printf("Enter Clue Text: ");
    getchar(); 
    if (scanf(" %[^\n]", newTreasure->clueText) != 1) {
        printf("Invalid input for Clue Text.\n");
        return;
    }

    printf("Enter Treasure Value: ");
    if (scanf("%d", &newTreasure->value) != 1) {
        printf("Invalid input for Treasure Value.\n");
        return;
    }
}


void updateLogFile(const char* huntID, const char* DIRpath, int operation){ 
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t) {
        perror("localtime failed");
        return;
    
    }

    char logpath[MAX_FILEPATH_SIZE];
    snprintf(logpath, sizeof(logpath) ,"%s/logged-hunt-%s.txt", DIRpath, huntID); logpath[MAX_FILEPATH_SIZE - 1] = '\0';
    int logfd = open(logpath, O_WRONLY | O_CREAT | O_APPEND,  S_IRUSR | S_IWUSR);
    if(logfd == -1){
        perror(" log error opening/creating the file\n");
        return;
    }
    else {
        if(operation == 1){
        char timestamp[100];
        strftime(timestamp, sizeof(timestamp), "Added Treasure at: %Y-%m-%d %H:%M:%S\n", t);
        write(logfd, timestamp, strlen(timestamp));
        }
        else if(operation == 2){
            char timestamp[100];
            strftime(timestamp, sizeof(timestamp), "Deleted Treasure at: %Y-%m-%d %H:%M:%S\n", t);
            write(logfd, timestamp, strlen(timestamp));
        }
    }

}


void addTreasure(const char* huntID){
    char DIRpath[MAX_PATH_SIZE];

struct stat st = {0};
if (stat("Hunts", &st) == -1) {
    if (mkdir("Hunts", 0755) == -1) {
        perror("Failed to create 'Hunts' directory");
        return;
    }
}

    sprintf(DIRpath, "%s/%s", "Hunts", huntID);

    DIR* Hunt = opendir(DIRpath);
    if (!Hunt) {
        // If the directory doesn't exist, try to create it
        if (mkdir(DIRpath, 0755) == -1) {
            perror("Failed to create directory");
            return;
        } else {
            printf("Directory '%s' created successfully.\n", huntID);
            Hunt = opendir(DIRpath); 
            if (!Hunt) {
                perror("Directory opened failed after creation");
                return;
            }
        }
    }

    
    char filen[100];
    printf("Enter the name of the treasure file:");
    scanf("%s", filen); printf("\n");
    char filepath[MAX_FILEPATH_SIZE];
    sprintf(filepath, "%s/%s.dat", DIRpath, filen);

    treasureInfo newTreasure;
    inputTreasureInfo(&newTreasure);

    int fd = open(filepath, O_RDWR | O_CREAT | O_APPEND , S_IRUSR | S_IWUSR);
    if(fd == -1){
        printf("error opening/creating the file\n");
        return;
    }

    ssize_t bytes_written = write(fd, &newTreasure, sizeof(treasureInfo));
    if (bytes_written == -1) {
        printf("Error writing to file");
        perror("Error writing to file");
        return ;
    }
    close(fd);

  
    updateLogFile(huntID, DIRpath, 1);

    char logpath[MAX_FILEPATH_SIZE]; 
    snprintf(logpath, sizeof(logpath), "%s/logged-hunt-%s.txt", DIRpath, huntID);

    char symlinkPath[MAX_FILEPATH_SIZE];  
    snprintf(symlinkPath, sizeof(symlinkPath), "logged_hunt-%s.txt", huntID); symlinkPath[MAX_FILEPATH_SIZE - 1] = '\0';

    unlink(symlinkPath);

    if (symlink(logpath, symlinkPath) == -1) {
        perror("Failed to create symlink for logged_hunt");
    } else {
        printf("Symlink created: %s -> %s\n", symlinkPath, logpath);
    }
   
    closedir(Hunt);
}

void printLoggedHuntInfo(const char* huntID, const char* dirPath) {
    char logPath[MAX_FILEPATH_SIZE];
    snprintf(logPath, sizeof(logPath), "%s/logged-hunt-%s.txt", dirPath, huntID);

    int logfd = open(logPath, O_RDONLY);
    if (logfd == -1) {
        perror("Could not open logged hunt file");
        return;
    }

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesRead = read(logfd, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        printf("Log file is empty or unreadable.\n");
        close(logfd);
        return;
    }
    buffer[bytesRead] = '\0';

    
    char* lastLine = buffer;
    char* current = buffer;

    while ((current = strchr(current, '\n')) != NULL) {
        current++;  
        if (*current != '\0') {
            lastLine = current;
        }
    }

    printf("Last modification: %s", lastLine);

    close(logfd);
}

void printFileInfo(const char* filepath) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("stat failed");
        return;
    }

    printf("File: %s\n", filepath);
    printf("Size: %ld bytes\n", st.st_size);

}


void printTreasure(const treasureInfo* t) {
    printf("ID: %d | User: %s | Lat: %.2f | Long: %.2f | Value: %d\n", 
           t->treasureID, t->userName, t->latitude, t->longitude, t->value);
    printf("Clue: %s\n\n", t->clueText);
}


int listTreasuresInFile(const char* filepath, const char* treaseureID) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file for reading");
        return 0;
    }

    treasureInfo t;
    ssize_t bytes_read;
    int result = 0;

    while (1) {
        bytes_read = read(fd, &t, sizeof(treasureInfo));
        
        if (bytes_read == -1) {
            perror("Error reading file");
            break;
        }
        
        if (bytes_read == 0) {  // EOF
            break;
        }
        
        if (bytes_read != sizeof(treasureInfo)) {
            printf("Warning: Incomplete treasure record (read %zd of %zu bytes)\n", 
                  bytes_read, sizeof(treasureInfo));
            break;
        }
        if(t.treasureID == atoi(treaseureID)){
            printTreasure(&t);
            result = 1;
            break;
        }
        
    }
    close(fd);
    return result;
}


void listAllTreasuresInFile(const char* filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file for reading");
        return;
    }

    treasureInfo t;
    ssize_t bytes_read;

    
    while (1) {
        bytes_read = read(fd, &t, sizeof(treasureInfo));
        
        if (bytes_read == -1) {
            perror("Error reading file");
            break;
        }
        
        if (bytes_read == 0) {  // EOF
            break;
        }
        
        if (bytes_read != sizeof(treasureInfo)) {
            printf("Warning: Incomplete treasure record (read %zd of %zu bytes)\n", 
                  bytes_read, sizeof(treasureInfo));
            break;
        }
        printf("Treasure with ID:%d\n", t.treasureID);
        
    }
    close(fd);
}


void listTreasure(const char* huntID){
    char DIRpath[MAX_PATH_SIZE];
    sprintf(DIRpath, "%s/%s", "Hunts", huntID);

    DIR* Hunt = opendir(DIRpath);
    if (!Hunt) {
        if (mkdir(DIRpath, 0755) == -1) {
            perror("Failed to create directory");
            return;
        } else {
            printf("Directory '%s' created successfully.\n", DIRpath);
            Hunt = opendir(DIRpath); // Try opening again
            if (!Hunt) {
                perror("Directory opened failed after creation");
                return;
            }
        }
    }

    printf("Hunt: %s\n", huntID);
    printLoggedHuntInfo(huntID, DIRpath);
    printFileInfo(DIRpath);

    struct dirent* entry;
    while((entry = readdir(Hunt)) != NULL){
        if(strstr(entry->d_name, ".dat")){
            char filepath[MAX_FILEPATH_SIZE];
            snprintf(filepath, sizeof(filepath), "%s/%s", DIRpath, entry->d_name);
            listAllTreasuresInFile(filepath);
        }

    }

    closedir(Hunt);
}


void viewTreasure(const char* huntID, const char* treasureID){
    char DIRpath[MAX_PATH_SIZE];
    sprintf(DIRpath, "%s/%s", "Hunts", huntID);

    DIR* Hunt = opendir(DIRpath);
    if(!Hunt){
        printf("Wrong hunt id introduced");
        perror("Directory not opened");
        return ;
    }

    struct dirent* entry;
    int existTreasure = 0;
    while((entry = readdir(Hunt)) != NULL){
        if(strstr(entry->d_name, ".dat")){
            char filepath[MAX_FILEPATH_SIZE];
            snprintf(filepath, sizeof(filepath), "%s/%s", DIRpath, entry->d_name);
            if(listTreasuresInFile(filepath, treasureID) == 1){
                existTreasure = 1;
                break;
            }
        }
    }
    if(existTreasure == 0){
        printf("Treasure id doesn't exists\n");
    }
}


void removeTreasure(const char* huntID, const char* treasureID) {
    char dirPath[MAX_PATH_SIZE];
    snprintf(dirPath, sizeof(dirPath), "Hunts/%s", huntID);

    DIR* dir = opendir(dirPath);
    if (!dir) {
        perror("Could not open hunt directory");
        return;
    }

    struct dirent* entry;
    int found = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".dat")) {
            char filepath[MAX_FILEPATH_SIZE];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirPath, entry->d_name);

            int fd = open(filepath, O_RDWR);
            if (fd == -1) {
                perror("Failed to open file");
                continue;
            }

            treasureInfo t;
            off_t read_offset = 0;
            off_t write_offset = 0;

            while (read(fd, &t, sizeof(treasureInfo)) == sizeof(treasureInfo)) {
                if (t.treasureID != atoi(treasureID)) {
                    if (read_offset != write_offset) {
                        lseek(fd, write_offset, SEEK_SET);
                        write(fd, &t, sizeof(treasureInfo));
                    }
                    write_offset += sizeof(treasureInfo);
                } else {
                    found = 1; 
                }
                read_offset += sizeof(treasureInfo);
                lseek(fd, read_offset, SEEK_SET);
            }

            if (found) {
                ftruncate(fd, write_offset);
                printf("Treasure %s removed from file: %s\n", treasureID, entry->d_name);
                updateLogFile(huntID, dirPath, 2);
                close(fd);
                break;
            }

            close(fd);
        }
    }

    closedir(dir);

    if (!found) {
        printf("Treasure with ID %s not found in hunt %s.\n", treasureID, huntID);
    }
}

void removeHunt(const char* huntID) {
    char dirPath[MAX_PATH_SIZE];
    snprintf(dirPath, sizeof(dirPath), "Hunts/%s", huntID);

    DIR* dir = opendir(dirPath);
    if (!dir) {
        perror("Could not open hunt directory");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char filePath[MAX_FILEPATH_SIZE];
        snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);
        if (remove(filePath) != 0) {
            perror("Failed to remove file inside hunt directory");
        }
    }
    closedir(dir);

    if (rmdir(dirPath) != 0) {
        perror("Failed to remove hunt directory");
        return;
    }

    char symlinkPath[MAX_FILEPATH_SIZE];
    snprintf(symlinkPath, sizeof(symlinkPath), "logged_hunt-%s.txt", huntID);
    if (unlink(symlinkPath) != 0) {
        perror("Failed to remove symbolic link (maybe it doesn't exist)");
    } else {
        printf("Removed symbolic link: %s\n", symlinkPath);
    }

    printf("Hunt %s has been removed.\n", huntID);
}


int main(int argc, char const *argv[]){
    if (argc < 3) {  
        printf("Usage: %s add <huntID>\n", argv[0]);
        return 1;
    }
    if(strcmp("add", argv[1]) == 0 && argc == 3){
        addTreasure(argv[2]);
    }

    if(strcmp("list", argv[1]) == 0 && argc == 3){ 
        listTreasure(argv[2]);
    }

    if(strcmp("view", argv[1]) == 0 && argc == 4){ 
        viewTreasure(argv[2], argv[3]);
    }

    if(strcmp("remove", argv[1]) == 0 && argc == 4 ){ 
        removeTreasure(argv[2], argv[3]);
    }

    if(strcmp("remove", argv[1]) == 0 && argc == 3 ){ 
        removeHunt(argv[2]);
    }

    return 0;
}
