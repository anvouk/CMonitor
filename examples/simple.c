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

#include "cmonitor/cm.h"

#ifdef _MSC_VER
#  pragma comment(lib, "CMonitor.lib")
#endif /* _MSC_VER */

#if defined(_DEBUG) || !defined(NDEBUG)
#  define malloc cm_malloc
#  define free   cm_free
#endif

void on_err(const char* msg)
{
	printf("error: %s\n", msg);
}

int main(int argc, char* argv[])
{

#if defined(_DEBUG) || !defined(NDEBUG)
	if (!cm_init(stdout, on_err))
		return 0;
#endif

	void* mem1 = malloc(100);
	void* mem2 = malloc(200);
	void* mem3 = malloc(300);
	free(mem1);

#if defined(_DEBUG) || !defined(NDEBUG)
	cm_print_stats();
#endif

	void* mem4 = malloc(400);
	free(mem2);
	free(mem4);
	free(mem3);

	/* Test errors */
	/*
	free(NULL);
	*/

#if defined(_DEBUG) || !defined(NDEBUG)
	cm_print_stats();
#endif

	getchar();
}