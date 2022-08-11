
# TDWMallocLogger
 A little library that enables extra logging information for allocation operations

## Requirements

To my knowledge the library should work just fine with any OSX / iOS versions currently available. 
In case of any issues, don't hesitate to reach me out directly at the.dreams.wind@gmail.com

## Use
Include malloc logger header anywhere in you source code and call the `tdw_enable(disable)_malloc_logger()` function:

```C
tdw_enable_malloc_logger();
free(malloc(8));
tdw_disable_malloc_logger();
```
Shortly after you will find that all allocation operations (`malloc`, `calloc`, `realloc` and `free`) are reported in standard output:
```bash
 MALLOC: Pointer: 0x6000027a00e0; Size: 8
   FREE: Pointer: 0x6000027a00e0; Size: 0
```

Additionally you may enable/disable recognition of Objective-C classes with `tdw_enable(disable)_objc_class_tracker()`. Be advised, however,
that Objective-C runtime doesn't recognize any classes on allocation, thus you can only see the class name when the instance is deallocated:
```Objective-C
tdw_enable_malloc_logger();
tdw_enable_objc_class_tracker();
[NSObject new];
tdw_disable_malloc_logger();
tdw_disable_objc_class_tracker();
```
Possible output:
```bash
 CALLOC: Pointer: 0x600000600080; Size: 16
   FREE: Pointer: 0x600000600080; Size: 0; Obj-C class: "NSObject"
```
