#ifndef ASSERT_H
#define ASSERT_H

#ifdef NDEBUG
#define assert(expr)
#else
#define assert(expr) ((expr) ? (void)0 : panic_assert(__FILE__, __LINE__, #expr))
#endif


#ifdef _cplusplus
extern "C" {
#endif
void assertion(const char* file, unsigned int line, const char* desc);
#ifdef _cplusplus
}
#endif

#endif
