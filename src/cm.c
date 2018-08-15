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
#include <stdarg.h>

#define C4C_PARAM_STRUCT_NAME cm_alloc_map
#define C4C_PARAM_PREFIX cm_map
#define C4C_PARAM_CONTENT void* block; size_t size; const char* filename; int line;
#include "c4c/linked_list/double_list_decl.inl"

#define C4C_PARAM_STRUCT_NAME cm_alloc_map
#define C4C_PARAM_PREFIX cm_map
#define C4C_PARAM_CONTENT void* block; size_t size; const char* filename; int line;
#include "c4c/linked_list/double_list_impl.inl"

static struct {
	uint32_t flags;
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

static void invoke_on_error(const char* format, ...)
{
	if (settings.on_error) {
		char buffer[128];
		strcpy_s(buffer, 128, format);

		va_list vl;
		va_start(vl, format);
		vprintf(buffer, vl);
		va_end(vl);

		settings.on_error(buffer);
	}
}

static int is_flag_set(uint32_t flag)
{
	return settings.flags & flag;
}

int cm_init(FILE* output, cm_error_fn on_error, uint32_t flags)
{
	settings.output = output;
	settings.on_error = on_error;
	if (!output) {
		invoke_on_error("[cmonitor initialization] cmonitor doesn't have a valid file output.");
		getchar();
		return 0;
	}
	settings.flags = flags;
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
	if (!out) {
		invoke_on_error("cm_get_stats(): out is an invalid pointer");
		return;
	}
	out->total_allocated = settings.info.total_allocated;
	out->total_freed = settings.info.total_freed;
	out->malloc_count = settings.info.malloc_count;
	out->free_count = settings.info.free_count;
}

void cm_get_leaks(cm_leak_info*** out_array, size_t* out_leaks_count)
{
	size_t delta, i;
	cm_alloc_map* il;
	cm_leak_info* leak;

	if (!out_leaks_count) {
		invoke_on_error("cm_get_leaks(): out_leaks_count is an invalid pointer");
		*out_array = NULL;
		return;
	}
	delta = settings.info.malloc_count - settings.info.free_count;
	if (delta == 0)
		goto zero_all;
	*out_array = malloc(sizeof(cm_leak_info*) * delta);
	if (!*out_array) {
		invoke_on_error("cm_get_leaks(): malloc failed");
		goto zero_all;
	}
	i = 0;
	c4c_list_foreach(&settings.map, il) {
		leak = malloc(sizeof(cm_leak_info));
		if (!leak) {
			invoke_on_error("cm_get_leaks(): malloc failed");
			free(*out_array);
			goto zero_all;
		}
		//strcpy(leak->filename, il->filename);
		leak->filename = il->filename;
		leak->line = il->line;
		leak->bytes = il->size;
		leak->address = il->block;
		(*out_array)[i] = leak;
		i++;
	}
	*out_leaks_count = delta;
	return;

zero_all:
	*out_array = NULL;
	*out_leaks_count = 0;
}

void cm_free_leaks_info(cm_leak_info** leak_array, size_t size)
{
	size_t i;

	if (!leak_array || size == 0)
		return;
	for (i = 0; i < size; i++) {
		free(leak_array[i]);
	}
	free(leak_array);
}

void* cm_malloc_(size_t size, const char* filename, int line)
{
	void* mem;
	cm_alloc_map* node;

	/* alloc new node */
	node = malloc(sizeof(cm_alloc_map));
	if (!node) {
		invoke_on_error("[%s:%d] malloc failed", get_filename(filename), line);
		exit(EXIT_FAILURE);
	}
	mem = malloc(size);
	if (!mem) {
		invoke_on_error("[%s:%d] malloc failed", get_filename(filename), line);
		exit(EXIT_FAILURE);
	}
	/* intialize new node */
	node->block = mem;
	node->size = size;
	node->filename = filename;
	node->line = line;
	/* increment stats */
	settings.info.total_allocated += size;
	settings.info.malloc_count++;
	cm_map_add(&settings.map, node);
	/* report allocation to output */
	fprintf(settings.output, "[%s:%d] malloc(%d)\n",
			get_filename(filename), line, size);
	return mem;
}

void cm_free_(void* mem, const char* filename, int line)
{
	cm_alloc_map* i;

	settings.info.free_count++;
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
	if (is_flag_set(CM_SIGNAL_ON_FREEING_NULL)) {
		if (!mem) {
			invoke_on_error("[%s:%d] attempt to free a NULL pointer.",
							get_filename(filename), line);
			return;
		}
	}
	if (is_flag_set(CM_SIGNAL_ON_FREEING_UNKNOWN)) {
		invoke_on_error("[%s:%d] attempt to free an unkwnown memory block.",
						get_filename(filename), line);
	}
}
