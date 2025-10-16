#include "minunit.h"
#include <lcthw/list_algos.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

char *values[] = {"XXXX", "1234", "abcd", "xjvef", "NDSS"};
#define NUM_VALUES 5

List *create_words()
{
    int i = 0;
    List *words = List_create();

    for(i = 0; i < NUM_VALUES; i++) {
        List_push(words, values[i]);
    }

    return words;
}

int is_sorted(List *words)
{
    LIST_FOREACH(words, first, next, cur) {
        if(cur->next && strcmp(cur->value, cur->next->value) > 0) {
            debug("%s %s", (char *)cur->value, (char *)cur->next->value);
            return 0;
        }
    }

    return 1;
}

char *test_bubble_sort()
{
    List *words = create_words();

    // should work on a list that needs sorting
    int rc = List_bubble_sort(words, (List_compare)strcmp);
    mu_assert(rc == 0, "Bubble sort failed.");
    mu_assert(is_sorted(words), "Words are not sorted after bubble sort.");

    // should work on an already sorted list
    rc = List_bubble_sort(words, (List_compare)strcmp);
    mu_assert(rc == 0, "Bubble sort of already sorted failed.");
    mu_assert(is_sorted(words), "Words should be sort if already bubble sorted.");

    List_destroy(words);

    // should work on an empty list
    words = List_create(words);
    rc = List_bubble_sort(words, (List_compare)strcmp);
    mu_assert(rc == 0, "Bubble sort failed on empty list.");
    mu_assert(is_sorted(words), "Words should be sorted if empty.");

    List_destroy(words);

    return NULL;
}

char *test_merge_sort()
{
    List *words = create_words();

    // should work on a list that needs sorting
    List *res = List_merge_sort(words, (List_compare)strcmp);
    mu_assert(is_sorted(res), "Words are not sorted after merge sort.");

    List *res2 = List_merge_sort(res, (List_compare)strcmp);
    mu_assert(is_sorted(res), "Should still be sorted after merge sort.");
    List_destroy(res2);
    List_destroy(res);

    List_destroy(words);
    return NULL;
}

// 生成一个大的随机字符串数组
List *create_big_words(int num)
{
    List *words = List_create();
    for(int i = 0; i < num; i++) {
        char *word = malloc(6);
        for(int j = 0; j < 5; j++) {
            word[j] = 'a' + rand() % 26;
        }
        word[5] = '\0';
        List_push(words, word);
    }
    return words;
}

char *test_perf_compare()
{
    srand(time(NULL));
    int num_words = 1000;     // 每次排序的元素数量
    int iterations = 100;     // 排序次数

    clock_t start, end;
    double bubble_time = 0.0;
    double merge_time = 0.0;

    // 测试冒泡排序
    start = clock();
    for(int i = 0; i < iterations; i++) {
        List *words = create_big_words(num_words);
        List_bubble_sort(words, (List_compare)strcmp);
        List_destroy(words);
    }
    end = clock();
    bubble_time = (double)(end - start) / CLOCKS_PER_SEC;

    // 测试归并排序
    start = clock();
    for(int i = 0; i < iterations; i++) {
        List *words = create_big_words(num_words);
        List *res = List_merge_sort(words, (List_compare)strcmp);
        List_destroy(words);
        List_destroy(res);
    }
    end = clock();
    merge_time = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\nPerformance results:\n");
    printf("Bubble Sort: %.3f seconds\n", bubble_time);
    printf("Merge Sort:  %.3f seconds\n", merge_time);
    printf("Speedup (bubble/merge) = %.2fx\n", bubble_time / merge_time);

    mu_assert(merge_time > 0.0 && bubble_time > 0.0, "Performance test did not run properly.");
    return NULL;
}


char *all_tests()
{
    mu_suite_start();

    mu_run_test(test_bubble_sort);
    mu_run_test(test_merge_sort);
    mu_run_test(test_perf_compare);

    return NULL;
}

RUN_TESTS(all_tests);