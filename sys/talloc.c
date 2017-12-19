/**
 **  TALLOC.C
 **
 **  lean_and_mean malloc()/free implementation
 **  by tom ehlert, te@drivesnapshot.de
 **
 **  please be careful:
 **
 **  this is only slightly tested and has NO ERRORCHECCKING AT ALL
 **  no internal checking. no stack checking. nothing !!
 **
 **  use it only, if your programs are already debugged !!
 **/

#ifndef _MSC_VER /* MSC has no brk/sbrk */

#include <stddef.h>
#include <string.h>

#ifdef __TURBOC__
extern unsigned __brklvl;
#define sbrk(x) ((void *)__brklvl)
#define brk(newbrk) \
	(((char *)(newbrk) > (char *)(&length) - 0x200) ? \
		-1 : \
		(__brklvl = (unsigned)(newbrk), 0))
#endif

#ifdef __WATCOMC__
#include <stdlib.h>
#define brk(newbrk) ((int)__brk((unsigned)(newbrk)))
#endif

#ifdef __GNUC__
#include <unistd.h>
static inline int brk(void *addr)
{
  char *brklvl = sbrk(0);
  int *res = sbrk((char *)addr - brklvl);
  return (res == (void *)-1) ? -1 : 0;
}
#endif

#define BUSY	    (sizeof(size_t)-1)	/* Bit set if memory block in use*/
#define isbusy(x)   ((x->length)&BUSY)


/**
*** _memavl()
***   return max. memory available
***   Q & D
**/

#ifdef DEBUG
extern int printf(const char *x, ...);
#define dbprintf(x) printf x
#else
#define dbprintf(x)
#endif
/*#define printf getch() != ' ' ? exit(0) : printf*/

typedef union {
  size_t length;
  char data[1];
} block;

void *malloc(size_t length)
{
  static	block *alloc_bottom = NULL;

  block	*akt, *next;
  block	*ltop = sbrk(0);

  if (alloc_bottom == NULL)
    alloc_bottom = ltop;

  length = (length + sizeof(size_t) + BUSY) & ~BUSY;

  akt = alloc_bottom;
  for (;;)
  {
    if (akt == ltop)
    {
      /* end of initialized memory */
      next = (block *)&akt->data[length];
      if (next < akt || brk(next) == -1)
	return NULL;
      break;
    }
    dbprintf(("follow [%x] = %x\n",akt, akt->length));
    next = (block *)(&akt->data[akt->length & ~BUSY]);
    if (isbusy(akt))
    {
      akt = next; /* next block	 */
    }
    else if (next == ltop || isbusy(next))
    {
      size_t size = akt->length;
      if (size >= length) /* try to split */
      {
	if (size > length) /* split */
        {
	  ((block *)&akt->data[length])->length =
	    size - length;
	}
	break;
      }
      akt = next; /* next block	 */
    }
    else
    {
      /* merge 2 blocks */
      akt->length += next->length;
    }
  }
  akt->length = length | BUSY;
  dbprintf(("set [%x] = %x\n",akt,akt->length));
  return &akt->data[sizeof(size_t)];
}

#ifdef __WATCOMC__
void *_nmalloc(unsigned length)
{
  return malloc(length);
}
#endif

/**
 ** reset busy-bit
 **/

void free(void *ptr)
{
  if (ptr)
    ((char *) ptr)[-sizeof(size_t)] &= ~BUSY;
}

#ifdef TALLOC_NEED_REALLOC

/* there isn't often need for realloc ;)
 */

void* realloc(void *ptr,size_t newlength)
{
  size_t oldlength = ((size_t*)ptr)[-1] & ~BUSY;
  void *newptr;

  newptr = malloc(newlength);

  if (newptr == NULL)
  {
    if (newlength <= oldlength)
      return ptr;
  }
  else
  {
    memcpy(newptr,ptr,oldlength);
  }

  free(ptr);
  return newptr;
}
#endif

#ifdef TEST
#include <stdio.h>

int gallocated = 0;

void *testalloc(size_t length)
{
  void *p;

  printf("alloc %x bytes - ",length);

  p = malloc(length);

  if (p) gallocated += length,printf("at %x - sum=%x\n",p,gallocated);
  else   printf("failed\n");
  return p;
}
void testfree(void* p)
{
  gallocated -= ((size_t*)p)[-1] & ~BUSY;
  printf("free %x \n",p);

  free(p);
}



main()
{
  char *s1,*s2,*s3,*s4,*s5,*s6;
  unsigned size;

  s1 = testalloc(1);
  s2 = testalloc(2);
  s3 = testalloc(0x10);
  s4 = testalloc(0x100);
  s5 = testalloc(0x1000);
  s6 = testalloc(0xfff0);

  testfree(s3);
  s3 = testalloc(6);
  testfree(s2);
  testalloc(8);

#ifdef __GNUC__
  for(size = 0xe0000000; size;)
#else
    for(size = 0xe000; size;)
#endif
      {
	if (testalloc(size))
	  ;
	else
	  size >>= 1;
      }


  testfree(s1);
  testfree(s5);
  testfree(s4);
  testfree(s6);

  return 1;
}
#endif

#endif
