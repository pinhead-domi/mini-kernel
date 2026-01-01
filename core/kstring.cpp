#include "kstring.h"
#include "types.h"
#include "memory.h"
#include "kernel_allocatpor.hpp"

extern KernelMemoryManager mem;

extern "C" void __assert_func(const char* file, int line, const char* func, const char* expr) {
    // Handle assertion failure in your kernel
    kprintf("Assertion failed: %s, function %s, file %s, line %d\n", 
            expr, func, file, line);
    
    // Panic or halt
    while(1) {
        __asm__ volatile("wfi"); // Wait for interrupt (RISC-V halt)
    }
}

extern "C" size_t strlen(const char *str)
{
  const char *pos = str;

  while (*pos)
  {
    ++pos;
  }

  return (pos - str);
}

extern "C" void *memcpy(void *dest, const void *src, size_t length)
{
  size_t i;
  size_t* s = (size_t*) src;
  size_t* d = (size_t*) dest;
  size_t num_large_copies = length / sizeof(size_t);
  size_t num_rest_copies = length % sizeof(size_t);
  for (size_t i = 0; i < num_large_copies; ++i)
  {
    *d++ = *s++;
  }
  uint8_t* s8 = (uint8_t*) s;
  uint8_t* d8 = (uint8_t*) d;
  for (i = 0; i < num_rest_copies; ++i)
  {
    *d8++ = *s8++;
  }

  return dest;
}

extern "C" void *memmove(void *dest, const void *src, size_t length)
{
  uint8_t* dest8 = (uint8_t*) dest;
  const uint8_t* src8 = (const uint8_t*) src;

  if (length == 0 || src == dest)
  {
    return dest;
  }

  if (src > dest)
  {
    // if src is _not_ before dest we can do a forward copy
    while (length--)
    {
      *dest8++ = *src8++;
    }
  }
  else
  {
    // if src is before dest we have to do a backward copy
    src8 += length - 1;
    dest8 += length - 1;

    while (length--)
    {
      *dest8-- = *src8--;
    }
  }

  return dest;
}

void *memccpy(void *dest, const void *src, uint8_t c, size_t length)
{
  uint8_t *dest8 = (uint8_t*) dest;
  const uint8_t *src8 = (const uint8_t*) src;

  if (length == 0)
  {
    return (void*) 0;
  }

  while (length--)
  {
    if ((*dest8++ = *src8++) == c)
    {
      return (void*) dest8;
    }
  }

  return (void *) 0;
}

extern "C" void *memset(void *block, uint8_t c, size_t size)
{
  if (size)
  {
    size_t i;
    size_t* d = (size_t*) block;
    size_t large_c = c;
    for (i = 0; i < sizeof(size_t); i++)
    {
      large_c = (large_c << 8) | c;
    }
    size_t num_large_copies = size / sizeof(size_t);
    size_t num_rest_copies = size % sizeof(size_t);
    for (i = 0; i < num_large_copies; ++i)
    {
      *d++ = large_c;
    }
    uint8_t* d8 = (uint8_t*) d;
    for (i = 0; i < num_rest_copies; ++i)
    {
      *d8++ = c;
    }
  }

  return block;
}

extern "C" char *strcpy(char *dest, const char* src)
{
  char *start = dest;

  for (; (*dest = *src); ++src, ++dest)
    ;

  return start;
}

extern "C" char *strncpy(char *dest, const char* src, size_t size)
{
  char *start = dest;
  int8_t fill = 0;

  while (size--)
  {
    if (fill)
    {
      *dest = 0;
    }
    else if ((*dest = *src) == 0)
    {
      fill = 1;
    }

    src++;
    dest++;
  }

  return start;
}

extern "C" char *strdup(const char *src)
{
  size_t size = strlen(src) + 1;
  char *dest = 0;

  if ((dest = (char*) mem.kmalloc((size) * sizeof(char))) == (char*) 0)
  {
    return (char*) 0;
  }

  return (char*) memcpy(dest, src, size);
}

extern "C" char *strcat(char *dest, const char*append)
{
  char *start = dest + strlen(dest);
  strcpy(start, append);
  return dest;
}

extern "C" char *strncat(char *dest, const char*append, size_t size)
{
  char* save = dest;

  if (size == 0)
  {
    return save;
  }

  while (*dest)
  {
    ++dest;
  }

  while (size--)
  {
    if ((*dest = *append++) == '\0')
    {
      break;
    }
    ++dest;
  }

  *dest = '\0';
  return save;
}

extern "C" size_t strlcat(char *dest, const char*append, size_t size)
{
  size_t count = size;
  const char*append_start = append;
  size_t done = 0;

  while (count != 0 && *dest != '\0')
  {
    --count;
    ++dest;
  }
  done = size - count;

  if (count == 0)
  {
    return done + strlen(append);
  }

  while (count--)
  {
    if ((*dest++ = *append) == '\0')
    {
      break;
    }
    ++append;
  }

  return done + (append - append_start) - 1;
}

extern "C" void bcopy(void *src, void* dest, size_t length)
{
  uint8_t* dest8 = (uint8_t*) dest;
  const uint8_t* src8 = (const uint8_t*) src;

  if (length == 0 || src == dest)
  {
    return;
  }

  if (src < dest)
  {
    // if src is before dest we can do a forward copy
    while (length--)
    {
      *dest8++ = *src8++;
    }
  }
  else
  {
    // if src is _not_ before dest we can do a forward copy
    src8 += length;
    dest8 += length;

    while (length--)
    {
      *dest8-- = *src8--;
    }
  }

}

extern "C" int32_t memcmp(const void *region1, const void *region2, size_t size)
{
  const uint8_t* b1 = (const uint8_t*)region1;
  const uint8_t* b2 = (const uint8_t*)region2;

  if (size == 0)
  {
    return 0;
  }

  while (size--)
  {
    if (*b1++ != *b2++)
    {
      return (*--b1 - *--b2);
    }
  }

  return 0;
}

extern "C" int32_t strcmp(const char *str1, const char *str2)
{
  if (str1 == str2)
  {
    return 0;
  }

  while ((*str1) && (*str2))
  {
    if (*str1 != *str2)
    {
      break;
    }
    ++str1;
    ++str2;
  }

  return (*(uint8_t *) str1 - *(uint8_t *) str2);
}

extern "C" int32_t strncmp(const char *str1, const char *str2, size_t n)
{
  while (n && (*str1) && (*str2))
  {
    if (*str1 != *str2)
    {
      break;
    }
    ++str1;
    ++str2;
    --n;
  }
  if (n == 0)
    return 0;
  else
    return (*(uint8_t *) str1 - *(uint8_t *) str2);
}

extern "C" int32_t bcmp(const void *region1, const void *region2, size_t size)
{
  const uint8_t* b1 = (const uint8_t*)region1;
  const uint8_t* b2 = (const uint8_t*)region2;

  if (size == 0)
  {
    return 0;
  }

  while (size--)
  {
    if (*b1++ != *b2++)
    {
      return (*--b1 - *--b2);
    }
  }

  return 0;
}

extern "C" void *memnotchr(const void *block, uint8_t c, size_t size)
{
  const uint8_t *b = (const uint8_t*) block;

  while (size--)
  {
    if (*b != c)
    {
      return (void *) b;
    }
    ++b;
  }

  return (void *) 0;
}

extern "C" void *memchr(const void *block, uint8_t c, size_t size)
{
  const uint8_t *b = (const uint8_t*) block;

  while (size--)
  {
    if (*b == c)
    {
      return (void *) b;
    }
    ++b;
  }

  return (void *) 0;
}

extern "C" void *memrchr(const void *block, uint8_t c, size_t size)
{
  const uint8_t *b = ((const uint8_t*) block + size - 1);

  while (size--)
  {
    if (*b == c)
    {
      return (void *) b;
    }
    --b;
  }

  return (void *) 0;
}

extern "C" char *strchr(const char* str, char c)
{
  do
  {
    if (*str == c)
    {
      return (char *) str;
    }
  } while (*++str);

  return (char *) 0;
}

extern "C" char *strrchr(const char* str, char c)
{
  uint32_t len = strlen(str);
  const char *pos = str + len; // goes to '\0'

  do
  {
    if (*--pos == c)
    {
      return (char *) pos;
    }
  } while (--len);

  return (char *) 0;
}

extern "C" char* strtok(char* str, const char* delimiters)
{
  static char* str_to_tok = 0;
  if (str != 0)
    str_to_tok = str;

  // no delimiters, so just return the rest-string
  if (delimiters == 0)
    return str_to_tok;

  if (str_to_tok == 0)
    return 0;

  // determine token start and end
  uint32_t tok_start = 0;
  uint32_t tok_end = -1;

  // find first char which is not one of the delimiters
  uint32_t str_pos = 0;
  for (str_pos = 0; str_to_tok[str_pos] != '\0'; str_pos++)
  {
    uint8_t char_is_delimiter = 0;

    uint32_t del_pos = 0;
    for (del_pos = 0; delimiters[del_pos] != '\0'; del_pos++)
    {
      if (str_to_tok[str_pos] == delimiters[del_pos])
      {
        char_is_delimiter = 1;
        break;
      }
    }

    if (char_is_delimiter == 0)
    {
      // this is the start char of the token
      tok_start = str_pos;
      break;
    }
  }

  // find next delimiter in the string
  for (str_pos = tok_start; str_to_tok[str_pos] != '\0'; str_pos++)
  {
    uint32_t del_pos = 0;
    for (; delimiters[del_pos] != '\0'; del_pos++)
    {
      if (str_to_tok[str_pos] == delimiters[del_pos])
      {
        // delimiter found!
        tok_end = str_pos;
        break;
      }
    }

    if (tok_end != -1U)
      break;
  }

  // create and return token:
  char* token = str_to_tok + tok_start;

  // update string
  if (tok_end == -1U)
  {
    // finished, no next token
    str_to_tok = 0;
  }
  else
  {
    str_to_tok[tok_end] = '\0';
    str_to_tok += tok_end + 1;
  }

  return token;
}

// converts a single digit into an
extern "C" char numToASCIIChar(uint8_t number)
{
  if (number <= 9)
    return 0x30 + number;

  if (number >= 0xa && number <= 0xf)
    return 0x61 + number - 0xa;

  // default value
  return '?';
}

extern "C" char* itoa(int value, char* str, int base)
{
  if (!str)
    return 0;

  int div = value;
  int mod;
  unsigned int str_pos = 0;

  while (div >= base)
  {
    mod = div % base;
    div /= base;
    str[str_pos++] = numToASCIIChar(mod);
  }
  str[str_pos++] = numToASCIIChar(div);
  if (value < 0)
    str[str_pos++] = '-';
  str[str_pos] = '\0';

  if (str_pos > 1)
  {
    uint32_t str_len = strlen(str);
    uint32_t i = 0;
    // switching the string
    for (i = 0; i < str_len / 2; i++)
    {
      char temp = str[str_len - 1 - i];
      str[str_len - 1 - i] = str[i];
      str[i] = temp;
    }
  }

  return str;
}

extern "C" uint32_t checksum(uint32_t* src, uint32_t nbytes)
{
  nbytes /= sizeof(uint32_t);
  uint32_t poly = 0xEDB88320;
  int bit = 0, nbits = 32;
  uint32_t res = 0xFFFFFFFF;

  for (uint32_t i = 0; i < nbytes; ++i)
    for (bit = nbits - 1; bit >= 0; --bit)
      if ((res & 1) != ((src[i] >> bit) & 1))
        res = (res >> 1) ^ poly;
      else
        res = (res >> 1) + 7;

  res ^= 0xFFFFFFFF;
  return res;
}
