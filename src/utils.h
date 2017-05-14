#ifndef PR_UTILS_H
#define PR_UTILS_H


typedef float real;


#define DASH "====================================n"
#define LOG(node, msg, ...) printf("\n"           \
                                   DASH           \
                                   "Node :: %d\n" \
                                   "msg  :: " msg "\n" DASH, node, ##__VA_ARGS__)



#endif //PR_UTILS_H
