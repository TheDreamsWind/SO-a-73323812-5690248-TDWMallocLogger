//
//  nomalloc_printf.c
//  TDWMallocLogger
//
//  Created by Aleksandr Medvedev on 11.08.2022.
//

#include <unistd.h> // no idea what it is, but STDOUT_FILENO is defined there
#include <stdarg.h>
#include <pthread.h>

extern void _simple_vdprintf(int, const char *, va_list);

void nomalloc_printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    _simple_vdprintf(STDOUT_FILENO, format, ap);
    va_end(ap);
}

void nomalloc_printf_sync(const char *format, ...) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    va_list ap;
    va_start(ap, format);
    pthread_mutex_lock(&mutex);
    _simple_vdprintf(STDOUT_FILENO, format, ap);
    pthread_mutex_unlock(&mutex);
    va_end(ap);
}
