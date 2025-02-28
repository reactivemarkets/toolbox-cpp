# String Handling

## Introduction

The `strncpy()` function may lead to buffer overruns when used incorrectly.

The purpose this article is to:

1.  explain why the function is so often misunderstood;
2.  offer an alternative perspective to promote safe usage.

## Buffer Overruns

The `strcpy()` function will result in a buffer overrun if the source string is larger than the
destination:

``` cpp
char buf[6];
strcpy(buf, "Aylett");
```

The problem here is that the zero terminator will be written beyond the end of the buffer:

    +---+---+---+---+---+---+
    | A | y | l | e | t | t |\0
    +---+---+---+---+---+---+ ^
      0   1   2   3   4   5   write overrun

Programmers often reach for `strncpy()` as a possible solution, believing it to be a "bounded"
version of `strcpy()`:

``` cpp
char buf[6];
strncpy(buf, "Aylett", sizeof(buf));
```

But there are subtle differences. In this case, the function simply stops appending characters when
the destination buffer is exhausted, and it does not truncate the destination string with a
zero-terminator. This may lead to a read overrun if the consumer assumes a zero-terminated (c-style)
string:

    +---+---+---+---+---+---+
    | A | y | l | e | t | t |
    +---+---+---+---+---+---+
      0   1   2   3   4   5

This often leaves programmers wondering why `strncpy()` has such a seemingly broken contract. In
order to answer this question, we need to stop thinking of `strncpy()` as a bounded version of
`strcpy()`, and consider the use-case it was designed for.

## Data Leaks

One of the advantages of C and C++ is that they allow programmers to layout their data structures in
contiguous regions of memory:

``` cpp
struct __attribute__((packed)) Person {
  char forename[64];
  char surname[64];
};
static_assert(sizeof(Person) == 128);
static_assert(std::is_pod_v<Person>);
```

These "packed" data structures are ideal for wire protocols and file formats. When populating such
structures, however, programmers must take care to avoid security vulnerabilities:

``` cpp
Person person;
strcpy(person.forename, "Mark");
strcpy(person.surname, "Aylett");
```

This is bad, because memory locations beyond the zero-terminator are leaked to the outside world:

    +---+---+---+---+---+---+-------+---+
    | M | a | r | k |\0 | ? |  ...  | ? |
    +---+---+---+---+---+---+-------+---+
      0   1   2   3   4   5    ...    63

If these memory locations happened to contain sensitive data, then your system is now vulnerable to
attack.

## Message Padding

To avoid exposing private memory locations, we must ensure that any unused gaps in our data
structures are suitably filled. What we need is a function that copies the source string to the
destination, and then fills any remaining bytes in the destination buffer with a padding:

``` cpp
void pstrcpy(char* dst, const char* src, size_t n) noexcept
{
    // Copy source to destination.
    size_t i{0};
    for (; i < n && src[i] != '\0'; ++i) {
       dst[i] = src[i];
    }
    // Pad remaining bytes with space.
    for (; i < n; ++i) {
        dst[i] = ' ';
    }
}
```

The API can be simplified by inferring the size of the destination buffer:

``` cpp
template <std::size_t SizeN>
inline void pstrcpy(char (&dst)[SizeN], const char* src) noexcept
{
    // ...
}
```

This function can then be called as follows:

``` cpp
char buf[6];
pstrcpy(buf, "Mark");
```

Which will result in a "space padded" string:

    +---+---+---+---+---+---+
    | M | a | r | k |   |   |
    +---+---+---+---+---+---+
      0   1   2   3   4   5

Note that it is technically more correct to refer to this string encoding as "space padded" rather
than "space terminated", because the string will not be "space terminated" when the source string
length is greater than or equal to the destination.

## Strncpy Contract

We are now ready to tackle the notorious `strncpy()` contract. The FreeBSD man page states that:

> `strncpy()` copies at most `len` characters from `src` into `dst`. If `src` is less than `len`
> characters long, the remainder of `dst` is filled with \`\\0’ characters.

Notice anything familiar about this? It is precisely the function described in the previous section,
except the pad character is now a zero (‘\\0’) rather than a space.

The following idiom is often used when zero termination is required:

``` cpp
char buf[6 + 1];
strncpy(buf, "Aylett", sizeof(buf) - 1);
buf[sizeof(buf) - 1] = '\0';
```

This idiom is neatly encapsulated by the following C++ template, which avoids common programming
errors regarding the buffer size:

``` cpp
template <std::size_t SizeN>
inline void zstrcpy(char (&dst)[SizeN], const char* src) noexcept
{
    strncpy(dst, src, SizeN - 1);
    dst[SizeN - 1] = '\0';
}
```

## Read Safety

Although we now have a better understanding on the `strncpy()` contract, we still need a solution
for reading these zero-padded strings without overrunning the buffer. Consider the following member
function:

``` cpp
const char* surname() const
{
    return surname_;
}
```

If `surname_` is a zero-padded string with no padded, then it will not be zero-terminated and the
unwary user will likely overrun the buffer when reading. Assuming that we do not want a
zero-terminated string, the cleanest solution is to use `std::string_view<>`:

``` cpp
std::string_view surname() const
{
    return {surname_, strnlen(surname_, sizeof(surname_))};
}
```

Again, the key point is to think of `strncpy()` as a function that copies a c-style string into a
fixed length buffer, and then zero pads any remaining bytes.

## Summary

In summary:

  - the `strncpy()` is a poorly named function that is often misunderstood;
  - before using `strncpy()`, decide whether you actually want a *zero-terminated string* or a
    *zero-padded fixed length string*;
  - If a zero-padded fixed length string is required, then use `strncpy()` to write the string and
    `std::string_view<>` to read the string;

One final word of advice: don't gloss over the details. Taking time to revisit the basics will often
lead to new insights and fresh perspectives.
