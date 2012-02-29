#ifndef _KERN_GCC_H_
#define _KERN_GCC_H_

#ifdef _KERN_

#define gcc_aligned(mult)       __attribute__((aligned (mult)))

#define gcc_packed              __attribute__((packed))

#define gcc_inline              __inline __attribute__((always_inline))

#define gcc_noinline            __attribute__((noinline))

#define gcc_noreturn            __attribute__((noreturn))

#endif /* _KERN_ */

#endif /* !_KERN_GCC_H_ */