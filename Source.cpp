#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

int main() {
    mkdir("files", S_IRWXU);
    chdir("files");
    mkdir("a_2", S_IRWXU);
    mkdir("a_2/b_1", S_IRWXU);
    mkdir("a_2/b_0", S_IRWXU);
    int fd_cbin = open("a_2/b_0/c_0.bin", O_WRONLY | O_CREAT, S_IRWXU);
    char random_data[105];
    for (int i = 0; i < 105; ++i) {
        random_data[i] = rand() % 256;
    }
    write(fd_cbin, random_data, 105);
    close(fd_cbin);
    int fd_c1bin = open("a_2/b_0/c_1.bin", O_WRONLY | O_CREAT, S_IRWXU);
    char random_data1[673];
    for (int i = 0; i < 673; ++i) {
        random_data1[i] = rand() % 256;
    }
    write(fd_c1bin, random_data, 673);
    close(fd_c1bin);
    int fd_c2txt = open("a_2/b_0/c_2.txt", O_WRONLY | O_CREAT, S_IRWXU);
    write(fd_c2txt, "cat", 3);
    close(fd_c2txt);
    mkdir("a_2/b_0/c_3", S_IRWXU);
    link("a_2/b_0/c_0.bin", "a_2/b_2.bin");
    symlink("a_2/b_0", "a_0");
    symlink("a_2/b_0/c_1.bin", "a_1.bin");

    return 0;
}