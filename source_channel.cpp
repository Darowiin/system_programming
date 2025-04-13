#include "check.hpp"
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <ctime>
#include <stack>
#include <chrono>
#include <iostream>

void left_player(int pipe_write, int pipe_read, int max_range) {
    srand(time(0) ^ getpid());
    int number = rand() % max_range + 1;

    check(write(pipe_write, &max_range, sizeof(int)));

    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer),
                       "Process %d picked the number: %d\n",
                       getpid(), number);
    write(STDOUT_FILENO, buffer, len);

    bool guessed = false;
    while (!guessed) {
        int value;
        ssize_t read_size = check(read(pipe_read, &value, sizeof(int)));
        if (read_size == 0) exit(0);

        if (value == number) {
            guessed = true;
        }

        check(write(pipe_write, &guessed, sizeof(guessed)));
    }
}

void right_player(int pipe_write, int pipe_read) {
    int max_range;
    int value = 0;
    ssize_t read_size = check(read(pipe_read, &max_range, sizeof(int)));
    if (read_size == 0) exit(0);

    std::stack<int> numbers;
    for (int i = 1; i <= max_range; ++i)
        numbers.push(i);

    bool guessed = false;
    int count = 0;
    while (!numbers.empty() && !guessed) {
        value = numbers.top();
        numbers.pop();

        check(write(pipe_write, &value, sizeof(value)));

        read_size = check(read(pipe_read, &guessed, sizeof(guessed)));
        if (read_size == 0) exit(0);

        if (!guessed) {
            ++count;
        }
    }

    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer),
                       "Process %d guessed the number: %d | Count: %d\n",
                       getpid(), value, count);
    write(STDOUT_FILENO, buffer, len);
}

void role_change(int game_count, int pipe_write, int pipe_read, int pid, int range_max) {
    auto start_time = std::chrono::high_resolution_clock::now();

    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "Game %d\n", game_count);
    write(STDOUT_FILENO, buffer, len);

    if (game_count % 2 != 0) {
        if (pid == 0) {
            left_player(pipe_write, pipe_read, range_max);
        } else {
            right_player(pipe_write, pipe_read);
        }
    } else {
        if (pid > 0) {
            left_player(pipe_write, pipe_read, range_max);
        } else {
            right_player(pipe_write, pipe_read);
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    len = snprintf(buffer, sizeof(buffer), "Time: %lld ms\n", duration);
    write(STDOUT_FILENO, buffer, len);
}

void start(int rounds, int range_max) {
    int fd_1[2], fd_2[2];
    check(pipe(fd_1));  // fd_1[0] – для чтения родителем, fd_1[1] – для записи от ребенка
    check(pipe(fd_2));  // fd_2[0] – для чтения ребенком, fd_2[1] – для записи от родителя

    int pid = check(fork());

    srand(pid ? 1000 : 100000);

    int pipe_write, pipe_read;
    if (pid == 0) {
        pipe_write = fd_1[1];
        pipe_read  = fd_2[0];
        close(fd_1[0]);
        close(fd_2[1]);
    } else {
        pipe_write = fd_2[1];
        pipe_read  = fd_1[0];
        close(fd_1[1]);
        close(fd_2[0]);
    }

    for (int game_count = 1; game_count <= rounds; ++game_count) {
        role_change(game_count, pipe_write, pipe_read, pid, range_max);
    }

    close(pipe_read);
    close(pipe_write);
}

int main() {
    start(20, 10000);
}
