# Contributing to Toolbox

We want to make contributing to this project as easy and transparent as possible. All contributions
are welcome.

## Code of Conduct

The code of conduct is described in [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md).

## Issues

We use GitHub issues to track public bugs. Please ensure your description is clear and has
sufficient instructions to be able to reproduce the issue.

## Commit Messages

"Re-establishing the context of a piece of code is wasteful. We can't avoid it completely, so our
efforts should go to reducing it [as much] as possible. Commit messages can do exactly that and as a
result, a *commit message shows whether a developer is a good collaborator*." -- Peter Hutterer

The seven rules of a great git commit message:

1. separate subject from body with a blank line;
2. limit the subject line to 50 characters;
3. capitalise the subject line;
4. do not end the subject line with a period;
5. use the imperative mood in the subject line;
6. wrap the body at 72 characters;
7. use the body to explain _what_ and _why_ vs. _how_.

See [How to Write a Git Commit Message](http://chris.beams.io/posts/git-commit/) by Chris Beams.

## Pull Requests

We actively welcome your pull requests.

1. fork the repo and create your branch from `master`;
2. follow coding style and best practice;
3. add unit-test coverage where possible;
4. ensure the test suite passes;
5. ensure that there are no package cycles;
6. update the documentation for API changes.

## Coding Style

Please follow [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines) for anything not
covered in this section.

### Filenames

- [CamelCase](https://en.wikipedia.org/wiki/CamelCase) filenames;
- header files have `.hpp` extension;
- source files have `.cpp` extension;
- test files have `.ut.cpp` extension.

### Formatting

- spaces instead of tabs;
- indent by 4 spaces;
- lines should not exceed 100 characters.

[ClangFormat](http://clang.llvm.org/docs/ClangFormat.html) should be used to format source-code
changes. The
[GitClangFormat](https://github.com/llvm-mirror/clang/blob/master/tools/clang-format/git-clang-format)
script integrates [ClangFormat](http://clang.llvm.org/docs/ClangFormat.html) with
[Git](https://git-scm.com/).

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

## License

By contributing to Toolbox, you agree that your contributions will be licensed under the [Apache 2.0
License](https://www.apache.org/licenses/LICENSE-2.0). A copy of the license is available in the
LICENSE file in the root directory of this source tree.
