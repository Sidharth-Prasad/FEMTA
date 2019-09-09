#ifndef HEADER_GAURD_MATHEMATICS
#define HEADER_GAURD_MATHEMATICS

#define constant __attribute__((const))

int gcd(int a, int b) constant;

#define square(X)				\
  ({ __typeof__(X) __x = (X);			\
    __x * __x;					\
  })

#define cube(X)					\
  ({ __typeof__(X) __x = (X);			\
    __x * __x * __x;				\
  })


#endif
