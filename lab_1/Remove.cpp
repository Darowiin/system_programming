#include <iostream>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
void remove_directory(const char *path);
void remove_file(const char *path) {
    struct stat statbuf;

    if (lstat(path, &statbuf) == -1) {
        perror("Ошибка получения информации о файле.");
        return;
    }
    if (S_ISLNK(statbuf.st_mode) || S_ISREG(statbuf.st_mode)) {
        if (unlink(path) == -1) {
            perror("Ошибка удаления файла.");
        }
    } else if (S_ISDIR(statbuf.st_mode)) {
        remove_directory(path);
    }
}

void remove_directory(const char *path) {
    struct dirent **namelist;
    int n;

    n = scandir(path, &namelist, NULL, alphasort);

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Ошибка получения директории.");
    }
    if (chdir(path) == -1) {
        perror("Ошибка смены директории.");
        return;
    }

    for (int i = 0; i < n; i++) {
        if (strcmp(namelist[i]->d_name, ".") || strcmp(namelist[i]->d_name, "..") == 0) {
            free(namelist[i]);
            continue;
        }

        struct stat statbuf;
        if (lstat(namelist[i]->d_name, &statbuf) == -1) {
            perror("Ошибка получения информации о файле.");
            free(namelist[i]);
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            remove_directory(namelist[i]->d_name);
        } else {
            remove_file(namelist[i]->d_name);
        }
        free(namelist[i]);
    }

    if (chdir(cwd) == -1) {
        perror("Ошибка возврата в исходную директорию.");
        free(namelist);
        return;
    }
    if (rmdir(path) == -1) {
        perror("Ошибка удаления директории.");
    }

    free(namelist);
}

int main() {
    remove_directory("files");
    return 0;
}