/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dlmalloc.h"

#include "malloc.h"
#include "private/bionic_prctl.h"
#include "private/libc_logging.h"

// Send dlmalloc errors to the log.
static void __bionic_heap_corruption_error(const char* function);
static void __bionic_heap_usage_error(const char* function, void* address);
#define PROCEED_ON_ERROR 0
#define CORRUPTION_ERROR_ACTION(m) __bionic_heap_corruption_error(__FUNCTION__)
#define USAGE_ERROR_ACTION(m,p) __bionic_heap_usage_error(__FUNCTION__, p)

// Bionic named anonymous memory declarations.
static void* named_anonymous_mmap(size_t length);
#define MMAP(s) named_anonymous_mmap(s)
#define DIRECT_MMAP(s) named_anonymous_mmap(s)

#if defined(COMPATIBILITY_RUNTIME_BUILD)
#define HAVE_MORECORE 0
#define HAVE_MREMAP 0
#endif

// Ugly inclusion of C file so that bionic specific #defines configure dlmalloc.
#include "../upstream-dlmalloc/malloc.c"

static void __bionic_heap_corruption_error(const char* function) {
  __libc_fatal("heap corruption detected by %s", function);
}

static void __bionic_heap_usage_error(const char* function, void* address) {
  __libc_fatal_no_abort("invalid address or address of corrupt block %p passed to %s",
               address, function);
  // So that debuggerd gives us a memory dump around the specific address.
  // TODO: improve the debuggerd protocol so we can tell it to dump an address when we abort.
  *((int**) 0xdeadbaad) = (int*) address;
}

static void* named_anonymous_mmap(size_t length) {
  void* map = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (map == MAP_FAILED) {
    return map;
  }
#ifndef COMPATIBILITY_RUNTIME_BUILD
  prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, map, length, "libc_malloc");
#endif
  return map;
}

// Since dlmalloc isn't the default, we'll leave this unimplemented for now. If
// we decide we need it later, we can fill it in.
size_t __mallinfo_narenas() {
  return 0;
}

size_t __mallinfo_nbins() {
  return 0;
}

struct mallinfo __mallinfo_arena_info(size_t aidx __unused) {
  struct mallinfo mi;
  memset(&mi, 0, sizeof(mi));
  return mi;
}

struct mallinfo __mallinfo_bin_info(size_t aidx __unused, size_t bidx __unused) {
  struct mallinfo mi;
  memset(&mi, 0, sizeof(mi));
  return mi;
}
