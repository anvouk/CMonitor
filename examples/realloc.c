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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* link to cmonitor.lib */
#ifdef _MSC_VER
#  define CMAPI __declspec(dllimport)
#  pragma comment(lib, "CMonitor.lib")
#endif /* _MSC_VER */

#include "cmonitor/cm.h"

/* enable monitoring */
#if defined(_DEBUG) || !defined(NDEBUG)
#  define malloc  cm_malloc
#  define free    cm_free
#  define calloc  cm_calloc
#  define realloc cm_realloc
#endif

static void on_err(int cm_err, const char* msg)
{
	switch (cm_err) {
		case CM_ERR_INFO:
			printf("[INFO] %s\n", msg);
			break;
		case CM_ERR_WARNING:
			printf("[WARNING] %s\n", msg);
			break;
		case CM_ERR_ERROR:
			printf("[ERROR] %s\n", msg);
			break;
		case CM_ERR_UB:
			printf("[UNKNOWN BEHAVIOR] %s\n", msg);
			getchar();
			exit(EXIT_FAILURE);
			break;

	}
}

static void print_leaks(cm_leak_info** leaks, size_t size)
{
	size_t i;
	cm_leak_info* leak;

	puts("=== leaks begin ===");
	for (i = 0; i < size; i++) {
		leak = leaks[i];
		printf("%d. [%s:%d] %d bytes (address: %#010x)\n", i + 1,
			   leak->filename, leak->line, leak->bytes, (unsigned)leak->address);
	}
	puts("=== leaks end =====\n");
}

static void leaks_check(void)
{
	size_t leaks_count;
	cm_leak_info** leaks = NULL;

	cm_get_leaks(&leaks, &leaks_count);
	print_leaks(leaks, leaks_count);

	cm_free_leaks_info(leaks, leaks_count);
}

int main(int argc, char* argv[])
{
	void *mem1;

	printf("cmonitor | %s | examples/realloc.c\n\n", CM_VERSION_STR);

#if defined(_DEBUG) || !defined(NDEBUG)
	if (!cm_init(stdout, on_err, CM_SIGNAL_ALL))
		return 0;
#endif

	mem1 = malloc(100);

#if defined(_DEBUG) || !defined(NDEBUG)
	cm_print_stats();
	leaks_check();
#endif

	mem1 = realloc(mem1, 300);
	free(mem1);

#if defined(_DEBUG) || !defined(NDEBUG)
	cm_print_stats();
	leaks_check();
#endif

	getchar();
}