/*
 * The MIT License
 *
 * Copyright 2018 Andrea Vouk.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef CM_CM_H
#define CM_CM_H

/**
 * @file
 */

#include "cmonitor/config.h"

#include <stdio.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
	Library types
------------------------------------------------------------------------------*/

/**
 * A possible memory leak or simply a block of memory that has not been 
 * deallocated yet.
 */
typedef struct cm_leak_info {
	const char* filename; /**< Filename where the memory allocation happened. */
	int line;             /**< File's line where the memory allocation happened. */
	size_t bytes;         /**< Allocated bytes. */
	void* address;        /**< Allocated memory address. DO NOT free manually. */
} cm_leak_info;

/**
 * The program's allocation/deallocation balance.
 */
typedef struct cm_stats {
	uint32_t total_allocated; /**< Total allocated bytes since the initialization 
	                               of the library till the end of the program. */
	uint32_t total_freed;     /**< Total freed bytes since the initialization 
	                               of the library till the end of the program. */
	uint32_t malloc_count;    /**< Number of times the malloc function has been 
	                               called since the initialization of the library 
	                               till the end of the program. */
	uint32_t free_count;      /**< Number of times the free function has been 
	                               called since the initialization of the library 
	                               till the end of the program. */
	uint32_t calloc_count;    /**< Number of times the calloc function has been
	                               called since the initialization of the library 
	                               till the end of the program. */
	uint32_t realloc_count;   /**< Number of times the realloc function has been
	                               called since the initialization of the library 
	                               till the end of the program. */
} cm_stats;

/**
 * Callback function prototype for errors/warnings/infos.
 */
typedef void(*cm_error_fn)(int cm_err, const char* msg);

/*------------------------------------------------------------------------------
	Initialization flags
------------------------------------------------------------------------------*/

/**
 * If set, call cm_error_fn upon attempting to call malloc() with size set to
 * zero (Undefined behavior).
 */
#define CM_SIGNAL_ON_MALLOC_SIZE_ZERO  0x0001

/**
 * If set, call cm_error_fn upon attempting to free a NULL pointer. Ignore 
 * otherwise.
 */
#define CM_SIGNAL_ON_FREEING_NULL      0x0010

/**
 * If set, call cm_error_fn upon attempting to free a valid(?) memory block that
 * has not been previously registered with cm_malloc.
 */
#define CM_SIGNAL_ON_FREEING_UNKNOWN   0x0020

/**
 * If set, call cm_error_fn upon attempting to call calloc() with size set to
 * zero (Undefined behavior).
 */
#define CM_SIGNAL_ON_CALLOC_SIZE_ZERO  0x0100

/**
 * If set, call cm_error_fn upon attempting to reallocate an
 * unregistered/unknown memory block.
 */
#define CM_SIGNAL_ON_REALLOC_UNKNOWN   0x1000

/**
 * If set, call cm_error_fn upon attempting to call realloc() with size set to
 * zero (Undefined behavior).
 */
#define CM_SIGNAL_ON_REALLOC_SIZE_ZERO 0x2000

/**
 * If set, call cm_error_fn whenever some irregularity happens.
 */
#define CM_SIGNAL_ALL \
	( \
		CM_SIGNAL_ON_MALLOC_SIZE_ZERO  | \
		CM_SIGNAL_ON_FREEING_NULL      | \
		CM_SIGNAL_ON_FREEING_UNKNOWN   | \
		CM_SIGNAL_ON_CALLOC_SIZE_ZERO  | \
		CM_SIGNAL_ON_REALLOC_UNKNOWN   | \
		CM_SIGNAL_ON_REALLOC_SIZE_ZERO   \
	)

/*------------------------------------------------------------------------------
	Error flags
------------------------------------------------------------------------------*/

/**
 * Info.
 */
#define CM_ERR_INFO    0

/**
 * The function will immediately return.
 */
#define CM_ERR_WARNING 1

/**
 * An error has occurred/will occur. The program will be terminated.
 */
#define CM_ERR_ERROR   2

/**
 * An undefined behavior situation has been reached. By default the program will
 * NOT be terminated and something bad will probably happen.
 */
#define CM_ERR_UB      3

/*------------------------------------------------------------------------------
	Library functions
------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if 0 /* auto-indent fix */
}
#endif

/**
 * Initialize the library. Call before any cm_malloc or cm_free calls.
 *
 * @param output    The output where allocations/deallocations infos will be
 *                  printed to.
 * @param on_error  On errors this function will be called unless set to NULL.
 *
 * @retval 0  On failure (is output a non-null parameter?)
 * @retval 1  On success.
 */
CMAPI int CMCALL cm_init(FILE* output, cm_error_fn on_error, uint32_t flags);

/**
 * Print to the output (previously set during the library initialization) the
 * current stats.
 */
CMAPI void CMCALL cm_print_stats(void);

/**
 * Get a snapshot of the current stats.
 *
 * @param out  The stats snapshot.
 */
CMAPI void CMCALL cm_get_stats(cm_stats* out);

/**
 * Get a cm_leak_info heap-allocated array of size out_leaks_count with all the
 * non-deallocated (yet) memory blocks infos.
 *
 * @param out_array        A pointer to a cm_leak_info** array.
 * @param out_leaks_count  The number of possible memory leaks and the size of
 *                         the out_array leaks array.
 *
 * @note Remember to call cm_free_leaks_info in order to properly free the
 *       array.
 */
CMAPI void CMCALL cm_get_leaks(cm_leak_info*** out_array, size_t* out_leaks_count);

/**
 * Free a cm_leak_info** array.
 *
 * @param leak_array  The leaks array.
 * @param size        The cm_leak_info**'s array size.
 */
CMAPI void CMCALL cm_free_leaks_info(cm_leak_info** leak_array, size_t size);

/**
 * A wrapper around C malloc used to store allocation infos.
 *
 * @note Use the cm_malloc macro by defining malloc as cm_malloc when you want
 *       to enable allocation checking.
 *
 * @param size        The amount of bytes to allocate.
 * @param filename    The filename where this function is getting called from.
 * @param line        The line where this function is getting called from.
 * @param is_realloc  Internal use (set to 1 by cm_realloc_ when allocating an
 *                    entirely new block).
 *
 * @return On success, a pointer to the memory block allocated by the function.
 *         NULL otherwise.
 */
CMAPI void* CMCALL cm_malloc_(size_t size, const char* filename, int line, int is_realloc);

/**
 * A wrapper around C free used to store deallocation infos.
 *
 * @note Use the cm_free macro by defining free as cm_free when you want to
 *       enable allocation checking.
 *
 * @param mem       A pointer to a previously allocated block of memory.
 * @param filename  The filename where this function is getting called from.
 * @param line      The line where this function is getting called from.
 */
CMAPI void CMCALL cm_free_(void* mem, const char* filename, int line);

/**
 * A wrapper around C calloc used to store allocation infos.
 *
 * @param num       Number of elements to allocate.
 * @param size      The size of each element.
 * @param filename  The filename where this function is getting called from.
 * @param line      The line where this function is getting called from.
 *
 * @return On success, a pointer to the memory block allocated by the function.
 *         NULL otherwise.
 */
CMAPI void* CMCALL cm_calloc_(size_t num, size_t size, const char* filename, int line);

/**
 * A wrapper around C realloc used to store reallocation infos.
 *
 * @note If mem is NULL, cm_malloc_ will be called instead.
 *
 * @param mem       Pointer to a memory block previously allocated. If NULL
 *                  allocates a new block of memory.
 * @param size      New size for the memory block, in bytes.
 * @param filename  The filename where this function is getting called from.
 * @param line      The line where this function is getting called from.
 *
 * @return On success, a pointer to the memory block allocated by the function.
 *         NULL otherwise.
 */
CMAPI void* CMCALL cm_realloc_(void* mem, size_t size, const char* filename, int line);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#define cm_malloc(size)       cm_malloc_ (size, __FILE__, __LINE__, 0)
#define cm_free(mem)          cm_free_   (mem, __FILE__, __LINE__)
#define cm_calloc(num, size)  cm_calloc_ (num, size, __FILE__, __LINE__)
#define cm_realloc(mem, size) cm_realloc_(mem, size, __FILE__, __LINE__)

#endif /* CM_CM_H */
