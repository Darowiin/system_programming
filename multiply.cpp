#include <iostream>
#include <vector>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <chrono>
#include <cstring>
#include <cstdlib>

const int NUM_THREADS = 4;

struct ThreadData {
    const double* A, *B;
    double* C;
    int N;
    int startRow;
    int endRow;
};

void* multiplyRows(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    for (int i = data->startRow; i < data->endRow; ++i) {
        for (int j = 0; j < data->N; ++j) {
            double sum = 0.0;
            for (int k = 0; k < data->N; ++k) {
                sum += data->A[i * data->N + k] * data->B[k * data->N + j];
            }
            data->C[i * data->N + j] = sum;
        }
    }
    return nullptr;
}

bool readMatrix(const char* filename, std::vector<double>& matrix, int& N) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        std::cerr << "Ошибка открытия файла: " << filename << std::endl;
        return false;
    }
    struct stat st;
    fstat(fd, &st);
    size_t totalSize = st.st_size;
    if (totalSize % sizeof(double) != 0) {
        std::cerr << "Неверный размер файла: " << filename << std::endl;
        close(fd);
        return false;
    }
    int elemCount = totalSize / sizeof(double);
    N = static_cast<int>(sqrt(elemCount));
    if (N * N != elemCount) {
        std::cerr << "Файл не соответствует квадратной матрице: " << filename << std::endl;
        close(fd);
        return false;
    }
    matrix.resize(N * N);
    read(fd, matrix.data(), totalSize);
    close(fd);
    return true;
}

bool compareMatrices(const std::vector<double>& A, const std::vector<double>& B, int N) {
    const double EPS = 1e-6;
    for (int i = 0; i < N * N; ++i) {
        if (std::abs(A[i] - B[i]) > EPS) {
            std::cerr << "Несовпадение в элементе " << i
                      << ": " << A[i] << " vs " << B[i] << std::endl;
            return false;
        }
    }
    return true;
}

void printMatrix(const std::vector<double>& M, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            std::cout.width(8);
            std::cout.precision(2);
            std::cout << std::fixed << M[i * N + j] << " ";
        }
        std::cout << "\n";
    }
}

void multiplyMatrices(const std::vector<double>& A, const std::vector<double>& B, int N) {
    std::vector<double> C_seq(N * N, 0.0);
    std::vector<double> C_par(N * N, 0.0);

    auto start_seq = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double sum = 0.0;
            for (int k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C_seq[i * N + j] = sum;
        }
    }
    auto end_seq = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_seq = end_seq - start_seq;

    std::vector<pthread_t> threads(NUM_THREADS);
    std::vector<ThreadData> args(NUM_THREADS);

    int rowsPerThread = N / NUM_THREADS;
    int extraRows = N % NUM_THREADS;
    int currentRow = 0;

    auto start_par = std::chrono::high_resolution_clock::now();
    for (unsigned int t = 0; t < NUM_THREADS; ++t) {
        int startRow = currentRow;
        int endRow = startRow + rowsPerThread + (t < extraRows ? 1 : 0);
        args[t] = { A.data(), B.data(), C_par.data(), N, startRow, endRow };
        pthread_create(&threads[t], nullptr, multiplyRows, &args[t]);
        currentRow = endRow;
    }
    for (auto& th : threads) {
        pthread_join(th, nullptr);
    }
    auto end_par = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_par = end_par - start_par;

    if (compareMatrices(C_seq, C_par, N)) {
        std::cout << "Проверка пройдена: результаты совпадают.\n";
    } else {
        std::cerr << "Ошибка: результаты различаются!\n";
    }

    if (N < 100) {
        std::cout << "\nРезультирующая матрица:\n";
        printMatrix(C_seq, N);
    } else {
        std::cout << "\nВремя последовательного умножения: " << time_seq.count() << " секунд\n";
        std::cout << "Время параллельного умножения (" << NUM_THREADS << " потока(ов)): "
                << time_par.count() << " секунд\n";
    }
}

int main() {
    {
        const char* fileA = "../mat1_small.bin";
        const char* fileB = "../mat2_small.bin";

        std::vector<double> A, B;
        int N1, N2;
        if (!readMatrix(fileA, A, N1) || !readMatrix(fileB, B, N2)) {
            return 1;
        }
        multiplyMatrices(A, B, N1);
    }
    {
        std::vector<double> A, B;
        int N1, N2;
        if (!readMatrix("../mat1_big.bin", A, N1) || !readMatrix("../mat2_big.bin", B, N2)) {
            return 1;
        }
        multiplyMatrices(A, B, N1);

        return 0;
    }
}
