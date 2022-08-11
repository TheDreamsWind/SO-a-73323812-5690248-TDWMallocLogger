//
//  nomalloc_printf.h
//  TDWMallocLogger
//
//  Created by Aleksandr Medvedev on 10.08.2022.
//

#ifndef nomalloc_printf_h
#define nomalloc_printf_h

#include <unistd.h> // no idea what it is, but STDOUT_FILENO is defined there
#include <stdarg.h>

extern void _simple_vdprintf(int, const char *, va_list);

void nomalloc_printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    _simple_vdprintf(STDOUT_FILENO, format, ap);
    va_end(ap);
}

#endif /* nomalloc_printf_h */
