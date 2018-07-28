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

#define CM_VERSION_MAJOR 1
#define CM_VERSION_MINOR 0
#define CM_VERSION_PATCH 0
#define CM_VERSION_STATE "stable"

#define CM_VERSION_STR \
	CM_STR(_C4C_VERSION_MAJOR) "." \
	CM_STR(_C4C_VERSION_MINOR) "." \
	CM_STR(_C4C_VERSION_PATCH) "-" \
	CM_VERSION_STATE

#define CM_VERSION_MAKE(maj, min, patch) \
	((maj) << 16) | ((min) << 8) | (patch))

#ifndef CMCALL
#  define CMCALL __cdecl
#endif /* !CMCALL */

#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef CM_BUILDING_DLL
#    ifdef __GNUC__
#      define CMAPI __attribute__ ((dllexport))
#    else
#      define CMAPI __declspec(dllexport)
#    endif /* CM_BUILDING_DLL */
#  else
#    ifdef CM_USE_INLINE
#      define CMAPI
#    else
#      ifdef __GNUC__
#        define CMAPI __attribute__ ((dllimport))
#      else
#        define CMAPI __declspec(dllimport)
#      endif /* __GNUC__ */
#    endif /* CM_USE_INLINE */
#  endif /* CM_BUILDING_DLL */
#else
#  if __GNUC__ >= 4
#    define CMAPI __attribute__ ((visibility ("default")))
#  else
#    define CMAPI
#  endif /* __GNUC__ >= 4 */
#endif /* defined(_WIN32) || defined(__CYGWIN__) */

#endif /* CM_CONFIG_H */
