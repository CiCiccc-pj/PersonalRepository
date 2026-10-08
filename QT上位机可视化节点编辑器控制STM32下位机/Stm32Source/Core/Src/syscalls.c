/**
  ******************************************************************************
  * @file    syscalls.c
  * @brief   使用 arm-none-eabi-gcc 构建时所需的少量运行时桩函数。
  *
  * 本机 Homebrew 版 arm-none-eabi-gcc 未附带 newlib（没有 libc.a），
  * 而 CMSIS 的 GCC 启动文件会调用 __libc_init_array，
  * 该符号通常由 newlib 提供，这里自行实现以完成链接。
  ******************************************************************************
  */

#include <stddef.h>

/* 由链接脚本 STM32F103C8Tx_FLASH.ld 提供 */
extern void (*__preinit_array_start[])(void);
extern void (*__preinit_array_end[])(void);
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);

/**
  * @brief  依次执行 .preinit_array 和 .init_array 中的初始化函数
  */
void __libc_init_array(void)
{
  size_t count;
  size_t i;

  count = (size_t)(__preinit_array_end - __preinit_array_start);
  for (i = 0; i < count; i++)
  {
    __preinit_array_start[i]();
  }

  count = (size_t)(__init_array_end - __init_array_start);
  for (i = 0; i < count; i++)
  {
    __init_array_start[i]();
  }
}

/* ---- 以下内存函数本应由 libc 提供，未附带 newlib 时需自行实现 ---- */

void *memset(void *dst, int value, size_t len)
{
  unsigned char *p = (unsigned char *)dst;

  while (len--)
  {
    *p++ = (unsigned char)value;
  }
  return dst;
}

void *memcpy(void *dst, const void *src, size_t len)
{
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;

  while (len--)
  {
    *d++ = *s++;
  }
  return dst;
}

void *memmove(void *dst, const void *src, size_t len)
{
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;

  if (d < s)
  {
    while (len--)
    {
      *d++ = *s++;
    }
  }
  else
  {
    d += len;
    s += len;
    while (len--)
    {
      *--d = *--s;
    }
  }
  return dst;
}

int memcmp(const void *a, const void *b, size_t len)
{
  const unsigned char *pa = (const unsigned char *)a;
  const unsigned char *pb = (const unsigned char *)b;

  while (len--)
  {
    if (*pa != *pb)
    {
      return (int)*pa - (int)*pb;
    }
    pa++;
    pb++;
  }
  return 0;
}
