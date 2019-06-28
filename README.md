***DEPRECATED:*** improved version available [here](https://github.com/QwertyQaz414/anv)

# CMonitor
Keep an eye on every heap operation that happens in your program and spot memory leaks.

## Features
- malloc(), free(), calloc() and realloc() supported.
- Detect where memory leaks happened.
- Spot allocation functions misusage (e.g. asking malloc() to allocate zero bytes)
- Quick and easy integration in your project.
- Exstensive documentation.
- No dependencies.

## Usage
1. Link the library to your project.
2. At your project's root include something like this:
```cpp
#include "cmonitor/cm.h"

/* enable monitoring */
#if defined(_DEBUG) || !defined(NDEBUG)
#  define malloc  cm_malloc
#  define free    cm_free
#  define calloc  cm_calloc
#  define realloc cm_realloc
#endif
```
3. Done! That's it!

## Examples
You can find more examples in the <a href="https://github.com/QwertyQaz414/CMonitor/tree/master/examples">examples folder</a>
