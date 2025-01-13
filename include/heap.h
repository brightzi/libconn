#ifndef C_HEAP_H_
#define C_HEAP_H_

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct heap_node_st {
    int value;
} heap_node_st;

typedef heap_node_st * heap_node_t;
typedef int (*heap_compare_func)(const heap_node_t lhs,  const heap_node_t rhs);

typedef struct heap_st {
    heap_node_st **array;
    int size;
    int capacity;
    heap_compare_func compare;
} heap_st;
typedef struct heap_st * heap_t;

static inline heap_t create_heap(int capacity, heap_compare_func compare) {
    heap_t heap = (heap_t)malloc(sizeof(heap_st));
    heap->array = (heap_node_t *)malloc(capacity * sizeof(heap_node_t));
    heap->size = 0;
    heap->capacity = capacity;
    heap->compare = compare;
    return heap;
}

// 上浮操作
static inline void heapify_up(heap_t heap, int index) {
    while (index && (heap->compare)(heap->array[index], heap->array[(index - 1) / 2])) {
        heap_node_t temp = heap->array[index];
        heap->array[index] = heap->array[(index - 1) / 2];
        heap->array[(index - 1) / 2] = temp;
        index = (index - 1) / 2;
    }
}

// 插入元素
static inline void heap_insert(heap_t heap, heap_node_t node) {
    if (heap->size == heap->capacity) {
        printf("Heap is full\n");
        return;
    }
    // printf("heap_insert: %p\n", node);
    heap->array[heap->size] = node;
    heap->size++;
    heapify_up(heap, heap->size - 1);
}

// 下沉操作
static inline void heapify_down(heap_t heap, int index) {
    int parent_index = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < heap->size && (heap->compare)(heap->array[left], heap->array[parent_index])) {
        parent_index = left;
    }
    if (right < heap->size && (heap->compare)(heap->array[right], heap->array[parent_index])) {
        parent_index = right;
    }
    if (parent_index != index) {
        heap_node_t temp = heap->array[index];
        heap->array[index] = heap->array[parent_index];
        heap->array[parent_index] = temp;
        heapify_down(heap, parent_index);
    }
}

static inline heap_node_t heap_top(heap_t heap) {
    if (heap->size == 0) {
        printf("Heap is empty\n");
        return NULL; // 表示堆为空
    }

    return heap->array[0];
}

// 删除最小元素
static inline void heap_pop(heap_t heap) {
    if (heap->size == 0) {
        printf("Heap is empty\n");
        return ; // 表示堆为空
    }
    printf("heap pop\n");
    heap->array[0] = heap->array[heap->size - 1];
    heap->array[heap->size - 1] = NULL;
    heap->size--;
    heapify_down(heap, 0);
    return ;
}

// 删除任意节点
static inline void heap_remove(heap_t heap, heap_node_t node) {
    int index = -1;
    for (int i = 0; i < heap->size; i++) {
        if (heap->array[i] == node) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("Node not found\n");
        return ;
    }

    printf("heap remove\n");
    if (index == heap->size -1) {
        heap->array[heap->size - 1] = NULL;
        heap->size--;
    } else {
        heap->array[index] = heap->array[heap->size - 1];
        heap->array[heap->size - 1] = NULL;
        heap->size--;
        heapify_down(heap, index);
        heapify_up(heap, index);
    }
}


// 释放堆的内存
static inline void free_heap(heap_t heap) {
    free(heap->array);
    free(heap);
}

#ifdef __cplusplus
}
#endif

#endif
