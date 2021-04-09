//
// Created by millie on 05/04/2021.
//

#ifndef SERAPHIM_TEST_HEADER_H
#define SERAPHIM_TEST_HEADER_H

#include <string.h>

#define TEST_FAILURE "TEST FAILURE"
#define TEST_SUCCESS NULL

#define TEST_ASSERT(cond, message) do { \
    if (!(cond)){              \
        return message;   \
    }                          \
} while (0)

#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT((ptr) != NULL)

#define STRINGIZE(s) #s
#define RUN_TEST(name) do { \
    const char * message = name(); \
    if (message == TEST_SUCCESS){ \
        passed_tests++;                        \
    } else { \
        if (strcmp(message, TEST_FAILURE) == 0){                  \
            printf("Test \"" STRINGIZE(name) "\" failed with no message\n");                    \
        } else {            \
            printf("Test \"" STRINGIZE(name) "\" failed with message \"%s\"\n", message); \
        }\
    }                       \
    total_tests++;                            \
} while (0)

#endif //SERAPHIM_TEST_HEADER_H
