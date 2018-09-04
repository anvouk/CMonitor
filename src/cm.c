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
#endif

#include "cmonitor/cm.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

/* make cm_alloc_map functions private */
#define C4C_FUNCTION(rettype, name, ...) \
	static rettype CMCALL name(__VA_ARGS__)

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

static void invoke_on_error(int err, const char* format, ...)
{
	if (!settings.on_error)
		return;

	char buffer[128];
	va_list vl;

	va_start(vl, format);
	vsprintf(buffer, format, vl);
	va_end(vl);

	settings.on_error(err, buffer);
}

#define notify(err, format, ...) \
	invoke_on_error(err, "[%s:%d] " format, get_filename(filename), line, __VA_ARGS__)

static int is_flag_set(uint32_t flag)
{
	return settings.flags & flag;
}

int cm_init(FILE* output, cm_error_fn on_error, uint32_t flags)
{
	settings.output = output;
	settings.on_error = on_error;
	settings.flags = flags;
	if (!output) {
		invoke_on_error(CM_ERR_WARNING,
						"cm_init(): cmonitor doesn't have a valid file output.");
		return 0;
	}
	settings.map.block = NULL;
	settings.map.size = 0;
	cm_map_init(&settings.map);
	memset(&settings.info, 0, sizeof(cm_stats));
	return 1;
}

void cm_print_stats(void)
{
	const char* msg =
		"\n /=========================\\\n"
		" |===    Quick Stats    ===|\n"
		" |=========================|\n"
		" |total alloc:      %.7d|\n"
		" |total free:       %.7d|\n"
		" |-------------------------|\n"
		" |total leaks:      %.7d|\n"
		" |                         |\n"
		" |total malloc():   %.7d|\n"
		" |total calloc():   %.7d|\n"
		" |-------------------------|\n"
		" |total free():     %.7d|\n"
		" |                         |\n"
		" |total realloc():  %.7d|\n"
		" \\=========================/\n\n";

	fprintf(settings.output, msg,
			settings.info.total_allocated,
			settings.info.total_freed,
			/*-------------------------*/
			settings.info.total_allocated - settings.info.total_freed,
			/*                         */
			settings.info.malloc_count,
			settings.info.calloc_count,
			/*-------------------------*/
			settings.info.free_count,
			/*                         */
			settings.info.realloc_count
	);
}

void cm_get_stats(cm_stats* out)
{
	if (!out) {
		invoke_on_error(CM_ERR_WARNING,
						"cm_get_stats(): out is an invalid pointer.");
		return;
	}
	out->total_allocated = settings.info.total_allocated;
	out->total_freed = settings.info.total_freed;
	out->malloc_count = settings.info.malloc_count;
	out->free_count = settings.info.free_count;
	out->calloc_count = settings.info.calloc_count;
	out->realloc_count = settings.info.realloc_count;
}

void cm_get_leaks(cm_leak_info*** out_array, size_t* out_leaks_count)
{
	size_t delta, i;
	cm_alloc_map* il;
	cm_leak_info* leak;

	if (!out_leaks_count) {
		invoke_on_error(CM_ERR_WARNING,
						"cm_get_leaks(): out_leaks_count is an invalid pointer.");
		*out_array = NULL;
		return;
	}
	delta = settings.info.malloc_count + settings.info.calloc_count
		- settings.info.free_count;
	if (delta == 0)
		goto zero_all;
	*out_array = malloc(sizeof(cm_leak_info*) * delta);
	if (!*out_array) {
		invoke_on_error(CM_ERR_ERROR,
						"cm_get_leaks(): internal malloc failed.");
		exit(EXIT_FAILURE);
	}
	i = 0;
	c4c_list_foreach(&settings.map, il) {
		leak = malloc(sizeof(cm_leak_info));
		if (!leak) {
			invoke_on_error(CM_ERR_ERROR,
							"cm_get_leaks(): internal malloc failed.");
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
	*leak_array = NULL;
}

void* cm_malloc_(size_t size, const char* filename, int line, int is_realloc)
{
	void* mem;
	cm_alloc_map* node;

	/* alloc new node */
	if (size == 0 && is_flag_set(CM_SIGNAL_ON_MALLOC_SIZE_ZERO))
		notify(CM_ERR_UB, "malloc called with 'size' zero. Undefined behavior.");
	node = malloc(sizeof(cm_alloc_map));
	if (!node) {
		notify(CM_ERR_ERROR, "internal malloc failed.");
		exit(EXIT_FAILURE);
	}
	mem = malloc(size);
	if (!mem) {
		/* check if realloc is calling malloc */
		if (is_realloc)
			notify(CM_ERR_ERROR, "(realloc) malloc failed.");
		else
			notify(CM_ERR_ERROR, "malloc failed.");
		exit(EXIT_FAILURE);
	}
	/* intialize new node */
	node->block = mem;
	node->size = size;
	node->filename = filename;
	node->line = line;
	/* update stats */
	settings.info.total_allocated += size;
	settings.info.malloc_count++;
	cm_map_add(&settings.map, node);
	/* report allocation to output */
	if (is_realloc)
		fprintf(settings.output, "[%s:%d] <%p> <realloc> malloc(%d)\n",
				get_filename(filename), line, mem, size);
	else
		fprintf(settings.output, "[%s:%d] <%p> malloc(%d)\n",
				get_filename(filename), line, mem, size);
	return mem;
}

void cm_free_(void* mem, const char* filename, int line)
{
	cm_alloc_map* i;

	settings.info.free_count++;
	c4c_list_foreach(&settings.map, i) {
		if (i->block == mem) {
			settings.info.total_freed += i->size;
			fprintf(settings.output, "[%s:%d] <%p> free(%d)\n",
					get_filename(filename), line, i->block, i->size);
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
			notify(CM_ERR_WARNING, "attempt to free a NULL pointer.");
			return;
		}
	}
	if (is_flag_set(CM_SIGNAL_ON_FREEING_UNKNOWN)) {
		notify(CM_ERR_WARNING, "attempt to free an unkwnown memory block.");
	}
}

void* cm_calloc_(size_t num, size_t size, const char* filename, int line)
{
	void* mem;
	cm_alloc_map* node;

	if (size == 0 && is_flag_set(CM_SIGNAL_ON_CALLOC_SIZE_ZERO))
		notify(CM_ERR_UB, "calloc called with param 'size' invalid value.");
	/* alloc new node */
	node = malloc(sizeof(cm_alloc_map));
	if (!node) {
		notify(CM_ERR_ERROR, "internal malloc failed.");
		exit(EXIT_FAILURE);
	}
	mem = calloc(num, size);
	if (!mem) {
		notify(CM_ERR_ERROR, "calloc failed.");
		exit(EXIT_FAILURE);
	}
	/* intialize new node */
	node->block = mem;
	node->size = num * size;
	node->filename = filename;
	node->line = line;
	/* update stats */
	settings.info.total_allocated += num * size;
	settings.info.calloc_count++;
	cm_map_add(&settings.map, node);
	/* report allocation to output */
	fprintf(settings.output, "[%s:%d] <%p> calloc(%d, %d) | total: %d\n",
			get_filename(filename), line, mem, num, size, num * size);
	return mem;
}

void* cm_realloc_(void* mem, size_t size, const char* filename, int line)
{
	void* new_mem;
	cm_alloc_map* node;
	size_t old_size = 0;
	int found = 0;

	if (!mem)
		return cm_malloc_(size, filename, line, 1);
	if (size == 0 && is_flag_set(CM_SIGNAL_ON_REALLOC_SIZE_ZERO))
		notify(CM_ERR_UB, "realloc called with 'size' zero. Undefined behavior.");
	new_mem = realloc(mem, size);
	if (!new_mem) {
		notify(CM_ERR_ERROR, "realloc failed.");
		exit(EXIT_FAILURE);
	}
	/* update memory */
	c4c_list_foreach(&settings.map, node) {
		if (node->block == mem) {
			node->block = new_mem;
			old_size = node->size;
			node->size = size;
			found = 1;
			break;
		}
	}
	if (!found && is_flag_set(CM_SIGNAL_ON_REALLOC_UNKNOWN))
		notify(CM_ERR_WARNING, "reallocated unknown memory block.");
	/* update stats */
	settings.info.total_allocated += (size - old_size);
	settings.info.realloc_count++;
	/* report reallocation to output */
	fprintf(settings.output, "[%s:%d] <%p> realloc(from: %d, to: %d) | diff: %d\n",
			get_filename(filename), line, new_mem, old_size, size, size - old_size);
	return new_mem;
}
