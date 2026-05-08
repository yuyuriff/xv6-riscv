#include <stdarg.h>

#include "param.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"

struct ring_buffer
{
    char *start;
    char *end;
    char *head;
    char *tail;
    uint64 size;
    uint64 n;
    struct spinlock lock;
};

static char rbuffer_buf[BUFPAGES * PGSIZE];
static struct ring_buffer rbuffer;

static char digits[] = "0123456789abcdef";

void
rbufferinit(void)
{
    initlock(&rbuffer.lock, "rbuffer");

    rbuffer.size = sizeof(rbuffer_buf);
    rbuffer.start = rbuffer_buf;
    rbuffer.end = rbuffer.start + rbuffer.size;

    rbuffer_buf[0] = '\n';
    rbuffer.head = rbuffer.start;
    rbuffer.tail = rbuffer.start + 1;
    rbuffer.n = 1;
}

static char*
get_next(char *pos)
{
    pos++;
    if (pos == rbuffer.end) {
        pos = rbuffer.start;
    }
    return pos;
}

static void
putc(char c)
{
    *rbuffer.tail = c;
    rbuffer.tail = get_next(rbuffer.tail);
    if (rbuffer.size == rbuffer.n) {
        rbuffer.head = get_next(rbuffer.head);
    } else {
        rbuffer.n++;
    }
}

void
rbufferputc(char c)
{
    acquire(&rbuffer.lock);
    putc(c);
    release(&rbuffer.lock);
}

static void
printint(long long xx, int base, int sign)
{
  char buf[20];
  int i;
  unsigned long long x;

  if(sign && (sign = (xx < 0)))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    putc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  putc('0');
  putc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    putc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

static void
rbuffer_printf(const char *fmt, va_list ap)
{
  int i, cx, c0, c1, c2;
  char *s;

  for(i = 0; (cx = fmt[i] & 0xff) != 0; i++){
    if(cx != '%'){
      putc(cx);
      continue;
    }
    i++;
    c0 = fmt[i+0] & 0xff;
    c1 = c2 = 0;
    if(c0) c1 = fmt[i+1] & 0xff;
    if(c1) c2 = fmt[i+2] & 0xff;
    if(c0 == 'd'){
      printint(va_arg(ap, int), 10, 1);
    } else if(c0 == 'l' && c1 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 2;
    } else if(c0 == 'u'){
      printint(va_arg(ap, uint32), 10, 0);
    } else if(c0 == 'l' && c1 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 2;
    } else if(c0 == 'x'){
      printint(va_arg(ap, uint32), 16, 0);
    } else if(c0 == 'l' && c1 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 2;
    } else if(c0 == 'p'){
      printptr(va_arg(ap, uint64));
    } else if(c0 == 'c'){
      putc(va_arg(ap, uint));
    } else if(c0 == 's'){
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        putc(*s);
    } else if(c0 == '%'){
      putc('%');
    } else if(c0 == 0){
      break;
    } else {
      putc('%');
      putc(c0);
    }

  }
}

void
pr_msg(const char *fmt, ...)
{   
    va_list ap;
    uint t;
    acquire(&tickslock);
    t = ticks;
    release(&tickslock);

    acquire(&rbuffer.lock);

    putc('[');
    printint(t, 10, 0);
    putc(']');
    putc(' ');

    va_start(ap, fmt);
    rbuffer_printf(fmt, ap);
    va_end(ap);

    putc('\n');

    release(&rbuffer.lock);
}

int
rbuffer_copyout(uint64 dst, int max_size)
{
    struct proc *p = myproc();
    char *cur;
    int copied;
    int total;

    if (max_size <= 0) {
        return -1;
    }

    acquire(&rbuffer.lock);

    cur = rbuffer.head;
    copied = 0;
    total = rbuffer.n;

    if (rbuffer.n == rbuffer.size) {
        int before_newline = 0;

        while (before_newline < total) {
            if (*cur == '\n') {
                cur = get_next(cur);
                before_newline++;
                break;
            }

            cur = get_next(cur);
            before_newline++;
        }

        total -= before_newline;
    }

    total = (total > max_size - 1) ? max_size - 1 : total;

    while (copied < total) {
        int to_end = rbuffer.end - cur;
        to_end = (to_end > total - copied) ? (total - copied) : to_end;

        if (copyout(p->pagetable, dst + copied, cur, to_end) < 0) {
            release(&rbuffer.lock);
            return -1;
        }   

        copied += to_end;
        cur += to_end;
        if (cur == rbuffer.end) {
            cur = rbuffer.start;
        }
    }

    char nullterm = '\0';
    if (copyout(p->pagetable, dst + copied, &nullterm, 1) < 0){
        release(&rbuffer.lock);
        return -1;
    }

    release(&rbuffer.lock);

    return copied;
}

uint64
sys_dmesg(void)
{
    uint64 buf;
    int size;

    argaddr(0, &buf);
    argint(1, &size);

    return rbuffer_copyout(buf, size);
}
