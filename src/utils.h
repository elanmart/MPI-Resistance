#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#ifndef PR_UTILS_H
#define PR_UTILS_H


/*
 *  logical operators
 */

#include <evdns.h>

#define not !
#define and &&
#define or  ||
#define is  ==


/*
 *  logging
 */
#define DASH "====================================n"
#define LOG(node, msg, ...) printf("\n"           \
                                   DASH           \
                                   "Node :: %d\n" \
                                   "msg  :: " msg "\n" DASH, node, ##__VA_ARGS__)

/*
 *  loops
 */
#define range(_name, _max) (int _name=0; _name<_max; _name++)


uint32_t unsafe_len(void* arr);


#endif //PR_UTILS_H
#pragma clang diagnostic pop