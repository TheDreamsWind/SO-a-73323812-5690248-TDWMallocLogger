//
//  malloc_logger.c
//  TDWMallocLogger
//
//  Created by Aleksandr Medvedev on 10.08.2022.
//

#include "malloc_logger.h"
#include "nomalloc_printf.h"

#include <pthread.h>
#include <objc/runtime.h>
#include <CoreFoundation/CoreFoundation.h>

#pragma mark - ObjC class record



pthread_mutex_t objc_class_records_mutex = PTHREAD_MUTEX_INITIALIZER;
CFMutableDictionaryRef objc_class_records;

bool is_malloc_logger_enabled(void);

void refresh_objc_class_list(void) {
    // Supress malloc logger while initiating Objc classes list
    bool should_reenable_malloc_logger = false;
    if (is_malloc_logger_enabled()) {
        tdw_disable_malloc_logger();
        should_reenable_malloc_logger = true;
    }
    
    pthread_mutex_lock(&objc_class_records_mutex);
    if (objc_class_records) {
        CFRelease(objc_class_records);
    }
        
    objc_class_records = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
    
    // The buffer needs to accomodate at least 26665 instances
    static const unsigned buffer_length = 100000;
    Class registered_classes[buffer_length];
    objc_getClassList(registered_classes, buffer_length);
    
    for (unsigned i = 0; i < buffer_length; ++i) {
        if (!registered_classes[i]) {
            break;
        }
        const Class class = registered_classes[i];
        const CFStringRef class_name = CFStringCreateWithCString(kCFAllocatorDefault, class_getName(class), kCFStringEncodingUTF8);
        CFDictionarySetValue(objc_class_records, class, class_name);
        CFRelease(class_name);
    }
    pthread_mutex_unlock(&objc_class_records_mutex);
    
    if (should_reenable_malloc_logger) {
        tdw_enable_malloc_logger();
    }
}

void tdw_disable_objc_class_tracker() {
    // Supress malloc logger while releasing Objc classes list
    bool should_reenable_malloc_logger = false;
    if (is_malloc_logger_enabled()) {
        tdw_disable_malloc_logger();
        should_reenable_malloc_logger = true;
    }
    
    if (!objc_class_records) {
        return;
    }
    CFRelease(objc_class_records);
    objc_class_records = NULL;
    
    if (should_reenable_malloc_logger) {
        tdw_enable_malloc_logger();
    }
}

void tdw_enable_objc_class_tracker() {
    refresh_objc_class_list();
}

#pragma mark - Malloc Logger

#define MALLOC_LOG_TYPE_ALLOCATE    2
#define MALLOC_LOG_TYPE_DEALLOCATE  4
#define MALLOC_LOG_TYPE_HAS_ZONE    8
#define MALLOC_LOG_TYPE_CLEARED     64

#define MALLOC_OP_MALLOC    (MALLOC_LOG_TYPE_ALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE)
#define MALLOC_OP_CALLOC    (MALLOC_OP_MALLOC | MALLOC_LOG_TYPE_CLEARED)
#define MALLOC_OP_REALLOC   (MALLOC_OP_MALLOC | MALLOC_LOG_TYPE_DEALLOCATE)
#define MALLOC_OP_FREE      (MALLOC_LOG_TYPE_DEALLOCATE | MALLOC_LOG_TYPE_HAS_ZONE)

typedef void(malloc_logger_t)(uint32_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uint32_t);
extern malloc_logger_t *malloc_logger;
malloc_logger_t* original_malloc_logger;

const char *alloc_type_name(uint32_t type) {
    switch (type) {
        case MALLOC_OP_MALLOC:
            return "MALLOC";
        case MALLOC_OP_CALLOC:
            return "CALLOC";
        case MALLOC_OP_REALLOC:
            return "REALLOC";
        case MALLOC_OP_FREE:
            return "FREE";
        default:
            return "UNKNOWN";
    }
}

bool log_objc_class(uint32_t type, void *ptr, unsigned size) {
    id objc_ptr = (id)ptr;
    
    pthread_mutex_lock(&objc_class_records_mutex);
    if (!objc_class_records || !objc_ptr) {
        pthread_mutex_unlock(&objc_class_records_mutex);
        return false;
    }
    
    Class objc_class = object_getClass(objc_ptr);
    if (!objc_class) {
        pthread_mutex_unlock(&objc_class_records_mutex);
        return false;
    }
    
    const CFStringRef class_name;
    const bool found = CFDictionaryGetValueIfPresent(objc_class_records, objc_class, (const void **)&class_name);
    pthread_mutex_unlock(&objc_class_records_mutex);
    
    if (found) {
        const static unsigned name_max_length = 256;
        char c_class_name[name_max_length];
        if (CFStringGetCString(class_name, c_class_name, name_max_length, kCFStringEncodingUTF8)) {
            const char *alloc_name = alloc_type_name(type);
            nomalloc_printf_sync("%s: ObjC class %s; Pointer: %p Size: %u\n", alloc_name, c_class_name, objc_ptr, size);
            return true;
        }
    }
    
    
    return false;
}

void my_malloc_logger(uint32_t type, uintptr_t param0, uintptr_t param1, uintptr_t param2,
                      uintptr_t param3, uint32_t frames_to_skip) {
    
    void *ptr = NULL;
    unsigned size = 0;
    
    switch (type) {
        case MALLOC_OP_MALLOC:
        case MALLOC_OP_CALLOC:
            ptr = (void *)param3;
            size = (unsigned)param1;
            break;
        case MALLOC_OP_REALLOC:
            ptr = (void *)param3;
            size = (unsigned)param2;
            break;
        case MALLOC_OP_FREE:
            ptr = (void *)param1;
            break;
    }
    
    if (!log_objc_class(type, ptr, size)) {
        const char *alloc_name = alloc_type_name(type);
        nomalloc_printf_sync("%s: Pointer: %p Size: %u\n", alloc_name, ptr, size);
    }
    
}

bool is_malloc_logger_enabled() {
    return malloc_logger == my_malloc_logger;
}

void tdw_enable_malloc_logger() {
    original_malloc_logger = malloc_logger;
    malloc_logger = my_malloc_logger;
}

void tdw_disable_malloc_logger() {
    if (!is_malloc_logger_enabled()) {
        return;
    }
    malloc_logger = original_malloc_logger;
}
