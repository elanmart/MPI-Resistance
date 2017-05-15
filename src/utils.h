#ifndef PR_UTILS_H
#define PR_UTILS_H

#include "common.h"


int randint(int low, int high);

#define DASH "=====================================\n"
#define LOG(msg, ...) printf("\n"                 \
                             DASH                 \
                             "Node :: %d     \n"  \
                             "msg  :: " msg "\n"  \
                             DASH,                \
                             this->ID_, ##__VA_ARGS__)

#endif //PR_UTILS_H
