#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>

const int NUM_THREADS = 4;

pthread_mutex_t mutex;
int lastIndex = -1;

struct SearchArgs {
    int* arr;
    int target;
    int start;
    int end;
};

void* searchLast(void* arg) {
    SearchArgs* a = static_cast<SearchArgs*>(arg);
    for (int i = a->start; i <= a->end; ++i) {
        if (a->arr[i] == a->target) {
            pthread_mutex_lock(&mutex);
            if (i > lastIndex) {
                lastIndex = i;
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    return nullptr;
}

bool readArrayFromFile(const char* filename, int*& arr, int& size) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        std::cerr << "Ошибка: не удалось получить размер файла" << std::endl;
        close(fd);
        return false;
    }

    size = st.st_size / sizeof(int);
    if (size == 0) {
        std::cerr << "Ошибка: файл пустой или некорректный" << std::endl;
        close(fd);
        return false;
    }

    arr = new int[size];
    ssize_t bytesRead = read(fd, arr, st.st_size);
    close(fd);

    if (bytesRead != st.st_size) {
        std::cerr << "Ошибка: не удалось полностью считать файл" << std::endl;
        delete[] arr;
        arr = nullptr;
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Использование: " << argv[0] << " <файл_массива> <значение_для_поиска>" << std::endl;
        return 1;
    }

    const char* filename = argv[1];
    int target = std::stoi(argv[2]);

    int* arr = nullptr;
    int size = 0;

    if (!readArrayFromFile(filename, arr, size)) {
        return 1;
    }

    pthread_mutex_init(&mutex, nullptr);

    pthread_t threads[NUM_THREADS];
    SearchArgs args[NUM_THREADS];

    int chunk = size / NUM_THREADS;
    int remainder = size % NUM_THREADS;
    int start = 0;

    for (int t = 0; t < NUM_THREADS; ++t) {
        int extra = (t < remainder) ? 1 : 0;
        int end = start + chunk + extra - 1;
        if (end >= size) end = size - 1;

        args[t] = { arr, target, start, end };

        pthread_create(&threads[t], nullptr, searchLast, &args[t]);
        start = end + 1;
    }

    for (int t = 0; t < NUM_THREADS; ++t) {
        pthread_join(threads[t], nullptr);
    }

    if (lastIndex != -1) {
        std::cout << "Последнее вхождение элемента " << target
                  << " находится на индексе " << lastIndex << std::endl;
    } else {
        std::cout << "Элемент " << target << " не найден в массиве" << std::endl;
    }

    pthread_mutex_destroy(&mutex);
    delete[] arr;
    return 0;
}
