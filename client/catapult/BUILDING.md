Building instructions
=====================

 * [linux building instructions](BUILDLIN.md)
 * [windows building instructions](BUILDWIN.md)

Sanitizers
----------

There are a few false positives when running sanitizers on targets
compiled with clang 9.
When building sanitizers, `sanitizer_blacklist.txt` file is used.

When running thread sanitizer, there is suppression required
for false positive in libc++ `shared_ptr`:
```
race:~weak_ptr
```
