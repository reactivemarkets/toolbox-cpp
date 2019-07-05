# Contributing

We want to make contributing to this project as easy and transparent as possible. All contributions
are welcome.

## Code of Conduct

The code of conduct is described in [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).

## Issues

We use GitHub issues to track public bugs. Please ensure your description is clear and has
sufficient instructions to be able to reproduce the issue.

## Commit Messages

"Re-establishing the context of a piece of code is wasteful. We can't avoid it completely, so our
efforts should go to reducing it \[as much\] as possible. Commit messages can do exactly that and as
a result, a *commit message shows whether a developer is a good collaborator*." -- Peter Hutterer

The seven rules of a great git commit message:

1. separate subject from body with a blank line;
2. limit the subject line to 50 characters;
3. capitalise the subject line;
4. do not end the subject line with a period;
5. use the imperative mood in the subject line;
6. wrap the body at 72 characters;
7. use the body to explain _what_ and _why_ vs. _how_.

See [How to Write a Git Commit Message](http://chris.beams.io/posts/git-commit/) by Chris Beams.

## Issue References

GitHub issue numbers should be referenced on the last line of the commit message. Keywords can be
used to [automatically close
issues](https://help.github.com/en/articles/closing-issues-using-keywords). For example:

```
The Subject Line

A more detailed description that explains what the change is,
why it is needed, and how it has been implemented.

Closes #123, #456
```

## Pull Requests

We actively welcome your pull requests.

1. fork the repo and create your branch from `master`;
2. follow coding style and best practice;
3. add unit-test coverage where possible;
4. ensure the test suite passes;
5. ensure that there are no package cycles;
6. update the documentation for API changes;
7. run `clang-format` on your changes.

## Coding Style

Follow [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines) for anything not covered in
this section.

[Go Proverbs](https://go-proverbs.github.io/) also offers some great advice:

- The bigger the interface, the weaker the abstraction.
- Make the zero value useful.
- A little copying is better than a little dependency.
- Clear is better than clever.
- Design the architecture, name the components, document the details.
- Documentation is for users.

### Filenames

- [CamelCase](https://en.wikipedia.org/wiki/CamelCase) filenames;
- header files have `.hpp` extension;
- source files have `.cpp` extension;
- test files have `.ut.cpp` extension.

### Formatting

- spaces instead of tabs;
- indent by 4 spaces;
- lines should not exceed 100 characters.

Use [ClangFormat](http://clang.llvm.org/docs/ClangFormat.html) to format source-code changes.
The `clang-format` build target formats the entire source tree:

```bash
$ cmake --build . --target clang-format
```

Alternatively, if you are using Unix Makefiles:

```bash
$ make clang-format
```

### Naming

- [CamelCase](https://en.wikipedia.org/wiki/CamelCase) for types and integer constants;
- [snake\_case](https://en.wikipedia.org/wiki/Snake_case) for functions, variables and non-integer
  constants;
- types and integer constants start with an upper-case character;
- trailing underscore for private data-members.

### Class Layout

The `public` section is first, followed by `protected` and then `private`. Within each section,
members are generally ordered as follows:

- big six;
- const functions;
- non-const functions;
- data.

Const functions precede non-const functions for two reasons:

1. it is the natural order imposed when a mutable interface extends an immutable interface;
2. mutators will most likely be placed alongside the private data-member section, which is generally
   more useful for maintainers.

### Assertions

Use `assert` statements to formalise contract and document assumptions.

### Comments

Comments documenting declarations should be full sentences, even if that seems a little redundant.
This approach makes them format well when extracted into Doxygen documentation.

### Exceptions

Exception messages should not be capitalised (unless beginning with proper nouns or acronyms) or end
with punctuation, since they are usually printed following other context. That is, use `throw
runtime_error{"something bad"}` not `throw runtime_error{"Something bad"}`, so that the example
below formats without a spurious capital letter mid-message.

```c++
TOOLBOX_ERROR << "exception: " << e.what();
```

### Logging

Long running applications and services should log exceptional conditions and life-cycle events.
Logging in library code, where the run-time context is often unknown, is generally frowned upon and
should be kept to a minimum. Log files should be kept clean and readable; they should not be used to
trace the normal flow of execution in production code.

## License

By contributing to Toolbox, you agree that your contributions will be licensed under the [Apache 2.0
License](https://www.apache.org/licenses/LICENSE-2.0). A copy of the license is available in the
[LICENSE.md](LICENSE.md) file in the root directory of this source tree.
