#define RELEASE_ASSERTIONS 1

#if (defined(NDEBUG) && RELEASE_ASSERTIONS)

#undef NDEBUG

#include <cassert>

#define NDEBUG

#else

#include <cassert>

#endif
