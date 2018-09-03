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

#ifndef CM_CONFIG_H
#define CM_CONFIG_H

/**
 * @file
 * 
 * This library uses <a href="https://semver.org/">Semantic Versioning 2.0.0</a>
 */

#define CM_STR_(x) #x
#define CM_STR CM_STR_

#define CM_VERSION_MAJOR 0
#define CM_VERSION_MINOR 3
#define CM_VERSION_PATCH 1
#define CM_VERSION_STATE "beta"

#define _CM_VERSION_STR(major, minor, patch) \
	CM_STR(major) "." \
	CM_STR(minor) "." \
	CM_STR(patch) "-" \
	CM_VERSION_STATE

#define CM_VERSION_STR \
	_CM_VERSION_STR(CM_VERSION_MAJOR, CM_VERSION_MINOR, CM_VERSION_PATCH)

#define CM_VERSION_MAKE(major, minor, patch) \
	((major) << 16) | ((minor) << 8) | (patch))

/*------------------------------------------------------------------------------
	Linker settings
------------------------------------------------------------------------------*/

#ifndef CMCALL
#  define CMCALL
#endif

#ifndef CMAPI
#  define CMAPI
#endif

/*------------------------------------------------------------------------------
	Other settings
------------------------------------------------------------------------------*/

#ifndef CM_THIS_FILE
#  define CM_THIS_FILE __FILE__
#endif

#ifndef CM_THIS_LINE
#  define CM_THIS_LINE __LINE__
#endif

#endif /* CM_CONFIG_H */
