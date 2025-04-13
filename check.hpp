#ifndef CHECK_HPP
#define CHECK_HPP 1

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

inline void error(const char* file, int line) {
    auto tmp = errno;//fprintf may fail, so we preserve errno
    fprintf(stderr, "%s (line %d) :", file, line);
    errno = tmp;
    perror(NULL);
    exit(EXIT_FAILURE);
}

inline int xcheck(int p, const char* file, int line) {
    if (p < 0) error(file, line);
    return p;
}

template<typename T>
inline T* xcheck(T* p, const char* file, int line) {
    if (p == nullptr) error(file, line);
    return p;
}

#define check(x) xcheck(x, __FILE__, __LINE__ )

inline int xcheck_except(int result, int except_error, const char* file, int line) {
    if (result < 0 && errno != except_error) {
        error(file, line);
    }
    return result;
}

#define check_except(x, except) xcheck_except((x), (except), __FILE__, __LINE__)

#endif // !CHECK_HPP