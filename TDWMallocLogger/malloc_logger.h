//
//  malloc_logger.h
//  ObjCPlayground
//
//  Created by Aleksandr Medvedev on 10.08.2022.
//

#ifndef malloc_logger_h
#define malloc_logger_h

void tdw_enable_malloc_logger(void);
void tdw_disable_malloc_logger(void);

void tdw_disable_objc_class_tracker(void);
void tdw_enable_objc_class_tracker(void);

#endif /* malloc_logger_h */
