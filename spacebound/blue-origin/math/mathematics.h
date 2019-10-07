#ifndef HEADER_GAURD_MATHEMATICS
#define HEADER_GAURD_MATHEMATICS

#define compiler_constant __attribute__((const))

int gcd(int a, int b) compiler_constant;

#define square(X)				\
  ({ __typeof__(X) __x = (X);			\
    __x * __x;					\
  })

#define cube(X)					\
  ({ __typeof__(X) __x = (X);			\
    __x * __x * __x;				\
  })

#define between(a, b, c)                                                \
  ({ __typeof__ (b) __b = (b);                                          \
    ((a) <= (__b) && ((__b) <= (c)));                                   \
  })

#endif
