#include <iostream>
#include <vector>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <chrono>

struct ThreadData {
    const double *A, *B;
    double *C;
    int N, row;
};

void* multiplyRow(void* arg) {
    ThreadData* d = static_cast<ThreadData*>(arg);
    int i = d->row, N = d->N;
    for(int j = 0; j < N; j++){
        double sum = 0.0;
        for(int k = 0; k < N; k++){
            sum += d->A[i*N + k] * d->B[k*N + j];
        }
        d->C[i*N + j] = sum;
    }
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <matrix1.bin> <matrix2.bin>\n";
        return 1;
    }
    const char* file1 = argv[1];
    const char* file2 = argv[2];

    int fd1 = open(file1, O_RDONLY), fd2 = open(file2, O_RDONLY);
    if (fd1 < 0 || fd2 < 0) {
        std::cerr << "Ошибка открытия файлов\n";
        return 1;
    }
    struct stat st1, st2;
    fstat(fd1, &st1);
    fstat(fd2, &st2);

    if (st1.st_size != st2.st_size || st1.st_size % sizeof(double) != 0) {
        std::cerr << "Неправильный размер файлов\n";
        return 1;
    }
    int totalElems = st1.st_size / sizeof(double);
    int N = static_cast<int>(std::sqrt(totalElems));
    if (N * N != totalElems) {
        std::cerr << "Размер файла не соответствует квадратной матрице\n";
        return 1;
    }

    std::vector<double> A(N*N), B(N*N), C_seq(N*N), C_par(N*N);
    read(fd1, A.data(), st1.st_size);
    read(fd2, B.data(), st2.st_size);
    close(fd1);
    close(fd2);

    auto start_seq = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            double sum = 0.0;
            for(int k = 0; k < N; k++){
                sum += A[i*N + k] * B[k*N + j];
            }
            C_seq[i*N + j] = sum;
        }
    }
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_seq = end_seq - start_seq;

    std::vector<pthread_t> threads(N);
    std::vector<ThreadData> args(N);
    for(int i = 0; i < N; i++){
        args[i] = {A.data(), B.data(), C_par.data(), N, i};
    }
    auto start_par = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < N; i++){
        pthread_create(&threads[i], nullptr, multiplyRow, &args[i]);
    }
    for(int i = 0; i < N; i++){
        pthread_join(threads[i], nullptr);
    }
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_par = end_par - start_par;

    std::cout << "Time (sequential): " << time_seq.count() << " s\n";
    std::cout << "Time (parallel): "   << time_par.count() << " s\n";
    return 0;
}
