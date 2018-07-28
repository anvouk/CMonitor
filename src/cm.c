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

/* Microsoft bs "warnings" */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif /* !_CRT_SECURE_NO_WARNINGS */

#include "cmonitor/cm.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define C4C_PARAM_LIST_CONTENT void* block; size_t size;
#define C4C_PARAM_LIST_PREFIX cm_map
#define C4C_PARAM_LIST_STRUCT_NAME cm_alloc_map
#include "c4c/linked_list/double_list_decl.inl"

#define C4C_PARAM_LIST_CONTENT void* block; size_t size;
#define C4C_PARAM_LIST_PREFIX cm_map
#define C4C_PARAM_LIST_STRUCT_NAME cm_alloc_map
#include "c4c/linked_list/double_list_impl.inl"

static struct {
	FILE* output;
	cm_error_fn on_error;

	cm_stats info;

	cm_alloc_map map;
} settings;

static const char* get_filename(const char* file)
{
#if defined(_WIN32) || defined(__CYGWIN__)
	return (strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file);
#else
	return (strrchr(file, '/') ? strrchr(file, '/') + 1 : file);
#endif /* _WIN32 */
}

static void invoke_on_error(const char* msg)
{
	if (settings.on_error)
		settings.on_error(msg);
}

int cm_init(FILE* output, cm_error_fn on_error)
{
	settings.output = output;
	settings.on_error = on_error;
	if (!output) {
		invoke_on_error("cmonitor doesn't have a valid output");
		return 0;
	}
	settings.map.block = NULL;
	settings.map.size = 0;
	cm_map_init(&settings.map);
	settings.info.malloc_count = 0;
	settings.info.free_count = 0;
	return 1;
}

void cm_print_stats(void)
{
	fprintf(settings.output, "\n=========================\n");
	fprintf(settings.output, "total alloc:  %d\ntotal free:   %d\ndelta:        %d\n\n",
			settings.info.total_allocated, settings.info.total_freed,
			settings.info.total_allocated - settings.info.total_freed);
	fprintf(settings.output, "malloc count: %d\nfree count:   %d\ndelta:        %d\n",
			settings.info.malloc_count, settings.info.free_count,
			settings.info.malloc_count - settings.info.free_count);
	fprintf(settings.output, "=========================\n\n");
}

void cm_get_stats(cm_stats* out)
{
	if (!out)
		return;
	out->total_allocated = settings.info.total_allocated;
	out->total_freed = settings.info.total_freed;
	out->malloc_count = settings.info.malloc_count;
	out->free_count = settings.info.free_count;
}

void* cm_malloc_(size_t size, const char* filename, int line)
{
	cm_alloc_map* node = malloc(sizeof(cm_alloc_map));
	if (!node) {
		invoke_on_error("malloc failed");
		exit(EXIT_FAILURE);
	}
	void* mem = malloc(size);
	if (!mem) {
		invoke_on_error("malloc failed");
		exit(EXIT_FAILURE);
	}
	node->block = mem;
	node->size = size;
	settings.info.total_allocated += size;
	settings.info.malloc_count++;
	cm_map_add(&settings.map, node);
	fprintf(settings.output, "[%s:%d] malloc(%d)\n",
			get_filename(filename), line, size);
	return mem;
}

void cm_free_(void* mem, const char* filename, int line)
{
	settings.info.free_count++;
	cm_alloc_map* i;
	c4c_list_foreach(&settings.map, i) {
		if (i->block == mem) {
			settings.info.total_freed += i->size;
			fprintf(settings.output, "[%s:%d] free(%d)\n",
					get_filename(filename), line, i->size);
			break;
		}
	}
	if (i != &settings.map) {
		cm_map_delete(i);
		free(i);
		free(mem);
		return;
	}
	char buf[128];
	if (!mem) {
		sprintf(buf, "attempt to free a NULL pointer at [%s:%d]",
				get_filename(filename), line);
		invoke_on_error(buf);
		return;
	}
	sprintf(buf, "attempt to free an unkwnown memory block at [%s:%d]",
			get_filename(filename), line);
	invoke_on_error(buf);
}
