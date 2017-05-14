#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#ifndef PR_UTILS_H
#define PR_UTILS_H


#include <evdns.h>


/*
 *  logical operators
 */
#define not !
#define and &&
#define or  ||
#define is  ==


/*
 *  logging
 */
#define DASH "=====================================\n"
#define LOG(node, msg, ...) printf("\n"                 \
                                   DASH                 \
                                   "Node :: %d     \n"  \
                                   "msg  :: " msg "\n"  \
                                   DASH,                \
                                   node, ##__VA_ARGS__)

/*
 *  loops
 */
#define range(_name, _max) (int _name = 0; _name < (_max); _name++)
#define each(_name, _arr) (int _name = 0; _arr[_name] != NULL; _name++)


/*
 *  functions
 */
void* safe_malloc(int32_t count, size_t dtype_size);
uint32_t unsafe_len(void* arr);
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))


#endif //PR_UTILS_H
#pragma clang diagnostic pop