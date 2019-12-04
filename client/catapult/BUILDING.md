Building instructions
=====================

 * [linux building instructions](BUILDLIN.md)
 * [windows building instructions](BUILDWIN.md)

Sanitizers
----------

There are a few false positives when running sanitizers on targets
compiled with clang 9.
When building sanitizers, `sanitizer_blacklist.txt` file is used.

When running thread sanitizer, there are following suppressions required:

 * for false positive in libc++ `shared_ptr`:
 * for false positive in boost logger, in server logger is always initialized from a single thread

```
race:~weak_ptr
race:global_logger::get()
```
