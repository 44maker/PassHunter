#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <regex.h>

void find_users_with_shell(char* filename, char* shell, char users[1024][32], int* user_count, int user_processed[1024]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* username = strtok(line, ":");
        for (int i = 0; i < 5; i++) {
            strtok(NULL, ":");  // Skip to the 6th field (shell field)
        }
        char* user_shell = strtok(NULL, ":");
        if (user_shell != NULL && strstr(user_shell, shell) != NULL) {
            int user_found = 0;
            for (int i = 0; i < *user_count; i++) {
                if (strcmp(users[i], username) == 0 && user_processed[i] == 0) {
                    user_found = 1;
                    user_processed[i] = 1; // Mark user as processed
                    break;
                }
            }

            if (!user_found) {
                strcpy(users[*user_count], username);
                (*user_count)++;
            }
        }
    }

    fclose(file);
}

void check_file_for_password(char* filename, char users[1024][32], int user_count) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return;
    }

    char line[256];
    regex_t regex;
    regcomp(&regex, "password|pass|p@ss|p@ssword", REG_EXTENDED | REG_ICASE);

    while (fgets(line, sizeof(line), file)) {
        for (int i = 0; i < user_count; i++) {
            if (strstr(line, users[i]) != NULL) {
                if (regexec(&regex, line, 0, NULL, 0) == 0) {
                    printf("\033[0;32m[+]Username: %s     Data: %s  ---File: %s\n\033[0m", users[i], line, filename);
                    break;  // Stop searching for other users in this line
                }
            }
        }
    }

    regfree(&regex);
    fclose(file);
}



void check_file_for_username(char* filename, char users[1024][32], int user_count) {
    if (strstr(filename, "config") != NULL) {
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            return;
        }

        char line[256];
        while (fgets(line, sizeof(line), file)) {
            for (int i = 0; i < user_count; i++) {
                if (strstr(line, users[i]) != NULL) {
                    printf("\033[0;32m[+]Data: %s  ---File: %s\n\033[0m", line, filename);
                    fclose(file);
                    return;
                }
            }
        }

        fclose(file);
    }
}

void explore_directory(char* path, char users[1024][32], int user_count) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path))) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char new_path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);
            explore_directory(new_path, users, user_count);
        } else {
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
            check_file_for_password(file_path, users, user_count);
            check_file_for_username(file_path, users, user_count);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    char users[1024][32];
    int user_count = 0;
    int user_processed[1024] = {0};

    find_users_with_shell("/etc/passwd", "zsh", users, &user_count, user_processed);
    find_users_with_shell("/etc/passwd", "fish", users, &user_count, user_processed);
    find_users_with_shell("/etc/passwd", "bash", users, &user_count, user_processed);
    find_users_with_shell("/etc/passwd", "sh", users, &user_count, user_processed);

    printf("__________                        ___ ___               __                 \n");
    printf("\\______   \\_____    ______ ______/   |   \\ __ __  _____/  |_  ___________  \n");
    printf(" |     ___/\\__  \\  /  ___//  ___/    ~    \\  |  \\/    \\   __\\/ __ \\_  __ \\ \n");
    printf(" |    |     / __ \\_\\___ \\ \\___ \\\\    Y    /  |  /   |  \\  | \\  ___/|  | \\/ \n");
    printf(" |____|    (____  /____  >____  >\\___|_  /|____/|___|  /__|  \\___  >__|    \n");
    printf("                \\/     \\/     \\/       \\/            \\/          \\/        \n");

    printf("===============================================================\n");
    printf("[-] Target User: ");
    for (int i = 0; i < user_count; i++) {
        printf("%s", users[i]);
        if (i < user_count - 1) {
            printf(",");
        }
    }
    printf("\n===============================================================\n");

    explore_directory(argv[1], users, user_count);

    return 0;
}