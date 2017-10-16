#ifndef _COMPILER_H_
#define _COMPILER_H_

/*
 * Copyright (C) 2001-2004 by egnite Software GmbH. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * For additional information see http://www.ethernut.de/
 */

/*
 * $Log$
 * Revision 1.12  2009/03/05 22:16:57  freckle
 * use __NUT_EMULATION instead of __APPLE__, __linux__, or __CYGWIN__
 *
 * Revision 1.11  2005/08/02 17:46:47  haraldkipp
 * Major API documentation update.
 *
 * Revision 1.10  2005/07/26 15:49:59  haraldkipp
 * Cygwin support added.
 *
 * Revision 1.9  2005/02/10 07:06:50  hwmaier
 * Changes to incorporate support for AT90CAN128 CPU
 *
 * Revision 1.8  2004/04/07 12:13:57  haraldkipp
 * Matthias Ringwald's *nix emulation added
 *
 * Revision 1.7  2004/03/18 15:51:45  haraldkipp
 * ICCAVR failed to compile
 *
 * Revision 1.6  2004/03/16 16:48:27  haraldkipp
 * Added Jan Dubiec's H8/300 port.
 *
 * Revision 1.5  2004/02/01 18:49:47  haraldkipp
 * Added CPU family support
 *
 */

#ifdef  __cplusplus
# define __BEGIN_DECLS  extern "C" {
# define __END_DECLS    }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif

/* Compiler Related Definitions */
#ifdef __CC_ARM                         /* ARM Compiler */
    #include <stdarg.h>
    #include <stddef.h>
    #define SECTION(x)                  __attribute__((section(x)))
    #define ATTRIBUTE_UNUSED            __attribute__((unused))
    #define ATTRIBUTE_USED              __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define WEAK                        __weak
    #define STATIC_INLINE               static __inline
    #define __API                       __declspec(dllexport)

#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  @ x
    #define ATTRIBUTE_UNUSED
    #define ATTRIBUTE_USED
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)                    PRAGMA(data_alignment=n)
    #define WEAK                        __weak
    #define STATIC_INLINE                   static inline
    #define __API

#elif defined (__GNUC__)                /* GNU GCC Compiler */
    /* the version of GNU GCC must be greater than 4.x */
    typedef __builtin_va_list   __gnuc_va_list;
    typedef __gnuc_va_list      va_list;
    #define va_start(v,l)       __builtin_va_start(v,l)
    #define va_end(v)           __builtin_va_end(v)
    #define va_arg(v,l)         __builtin_va_arg(v,l)

    #define SECTION(x)                  __attribute__((section(x)))
    #define ATTRIBUTE_UNUSED            __attribute__((unused))
    #define ATTRIBUTE_USED              __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define WEAK                        __attribute__((weak))
    #define STATIC_INLINE               static __inline
    #define __API
#elif defined (__ADSPBLACKFIN__)        /* for VisualDSP++ Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  __attribute__((section(x)))
    #define ATTRIBUTE_UNUSED            __attribute__((unused))
    #define ATTRIBUTE_USED              __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define WEAK                        __attribute__((weak))
    #define STATIC_INLINE               static inline
    #define __API
#elif defined (_MSC_VER)
    #include <stdarg.h>
    #define SECTION(x)
    #define ATTRIBUTE_UNUSED
    #define ATTRIBUTE_USED
    #define ALIGN(n)                    __declspec(align(n))
    #define WEAK
    #define STATIC_INLINE               static __inline
    #define __API
#elif defined (__TI_COMPILER_VERSION__)
    #include <stdarg.h>
    /* The way that TI compiler set section is different from other(at least
     * GCC and MDK) compilers. See ARM Optimizing C/C++ Compiler 5.9.3 for more
     * details. */
    #define SECTION(x)
    #define ATTRIBUTE_UNUSED
    #define ATTRIBUTE_USED
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)
    #define WEAK
    #define STATIC_INLINE                   static inline
    #define __API
#else
    #error not supported tool chain
#endif

#ifdef OS_CFG_HEAP
#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_START    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_START    (__segment_end("HEAP"))
#elif  defined (__GNUC__)                /* GNU GCC Compiler */
extern int __bss_end;
#define HEAP_START    (&__bss_end)
#else
    #error HEAP_START error
#endif
#endif /* OS_CFG_HEAP */

#endif // _COMPILER_H_
