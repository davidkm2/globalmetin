/******************************************************************************
 * Header: SecureEngineCustomVMs_BorlandC_inline.h
 * Description: Borland C++ inline assembly macros definitions
 *
 * Author/s: Oreans Technologies 
 * (c) 2014 Oreans Technologies
 *
 * --- File generated automatically from Oreans VM Generator (19/8/2014) ---
 ******************************************************************************/

/***********************************************
 * Definition as inline assembly
 ***********************************************/

#ifdef PLATFORM_X32

#ifndef VM_TIGER_WHITE_START
#define VM_TIGER_WHITE_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x64, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_WHITE_END
#define VM_TIGER_WHITE_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xF4, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_RED_START
#define VM_TIGER_RED_START                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x65, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_RED_END
#define VM_TIGER_RED_END                                   __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xF5, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_BLACK_START
#define VM_TIGER_BLACK_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x66, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_BLACK_END
#define VM_TIGER_BLACK_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xF6, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_WHITE_START
#define VM_FISH_WHITE_START                                __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x6A, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_WHITE_END
#define VM_FISH_WHITE_END                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xFA, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_RED_START
#define VM_FISH_RED_START                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x6C, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_RED_END
#define VM_FISH_RED_END                                    __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xFC, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_BLACK_START
#define VM_FISH_BLACK_START                                __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x6E, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_BLACK_END
#define VM_FISH_BLACK_END                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xFE, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_WHITE_START
#define VM_PUMA_WHITE_START                                __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x70, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_WHITE_END
#define VM_PUMA_WHITE_END                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x00, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_RED_START
#define VM_PUMA_RED_START                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x72, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_RED_END
#define VM_PUMA_RED_END                                    __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x02, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_BLACK_START
#define VM_PUMA_BLACK_START                                __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x74, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_BLACK_END
#define VM_PUMA_BLACK_END                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x04, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_WHITE_START
#define VM_SHARK_WHITE_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x76, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_WHITE_END
#define VM_SHARK_WHITE_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x06, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_RED_START
#define VM_SHARK_RED_START                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x78, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_RED_END
#define VM_SHARK_RED_END                                   __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x08, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_BLACK_START
#define VM_SHARK_BLACK_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x7A, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_BLACK_END
#define VM_SHARK_BLACK_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0A, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_MUTATE_ONLY_START
#define VM_MUTATE_ONLY_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x10, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_MUTATE_ONLY_END
#define VM_MUTATE_ONLY_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x11, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#endif

#ifdef PLATFORM_X64

#ifndef VM_TIGER_WHITE_START
#define VM_TIGER_WHITE_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x67, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_WHITE_END
#define VM_TIGER_WHITE_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xF7, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_RED_START
#define VM_TIGER_RED_START                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x68, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_RED_END
#define VM_TIGER_RED_END                                   __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xF8, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_BLACK_START
#define VM_TIGER_BLACK_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x69, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_TIGER_BLACK_END
#define VM_TIGER_BLACK_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xF9, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_WHITE_START
#define VM_FISH_WHITE_START                                __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x6B, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_WHITE_END
#define VM_FISH_WHITE_END                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xFB, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_RED_START
#define VM_FISH_RED_START                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x6D, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_RED_END
#define VM_FISH_RED_END                                    __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xFD, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_BLACK_START
#define VM_FISH_BLACK_START                                __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x6F, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_FISH_BLACK_END
#define VM_FISH_BLACK_END                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0xFF, 0x01, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_WHITE_START
#define VM_PUMA_WHITE_START                                __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x71, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_WHITE_END
#define VM_PUMA_WHITE_END                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x01, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_RED_START
#define VM_PUMA_RED_START                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x73, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_RED_END
#define VM_PUMA_RED_END                                    __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x03, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_BLACK_START
#define VM_PUMA_BLACK_START                                __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x75, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_PUMA_BLACK_END
#define VM_PUMA_BLACK_END                                  __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x05, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_WHITE_START
#define VM_SHARK_WHITE_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x77, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_WHITE_END
#define VM_SHARK_WHITE_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x07, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_RED_START
#define VM_SHARK_RED_START                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x79, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_RED_END
#define VM_SHARK_RED_END                                   __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x09, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_BLACK_START
#define VM_SHARK_BLACK_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x7B, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_SHARK_BLACK_END
#define VM_SHARK_BLACK_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x0B, 0x02, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_MUTATE_ONLY_START
#define VM_MUTATE_ONLY_START                               __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x10, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#ifndef VM_MUTATE_ONLY_END
#define VM_MUTATE_ONLY_END                                 __emit__ (0xEB, 0x10, 0x57, 0x4C, 0x20, 0x20, 0x11, 0x00, 0x00, 0x00, \
                                                                     0x00, 0x00, 0x00, 0x00, 0x57, 0x4C, 0x20, 0x20);
#endif

#endif

