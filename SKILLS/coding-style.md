# Additional C++ Coding Style Rules

These rules supplement `docs/coding-style.md`.

- Indent source files under `src/` with tabs, not spaces. A tab represents four
  columns; use spaces only to align continuation lines.
- Do not use anonymous namespaces (`namespace {}`). Put file-local helpers in
  the appropriate named namespace and declare them `static`.
- Define constants used by `.cpp` files as `static constexpr` in the relevant
  `.hpp` file. Class-specific constants belong in the class.
- Put `///` documentation immediately above function definitions in `.cpp`
  files, not above declarations in `.hpp` files. Omit comments that only repeat
  the code, including namespace closing comments.
- Initialize class member variables in constructor initializer lists or
  constructor bodies, not with default member initializers in `.hpp` files.
  `static constexpr` constants are the necessary exception.
- Do not use raw pointers for ordinary program state or APIs. Use references,
  values, `std::optional`, or indices as appropriate. Preserve pointer types
  only where a required external interface mandates them, such as `main`.
- Use postfix increment and decrement (`value++`, `value--`) instead of prefix
  forms.
- Keep a function signature or other expression on one line when it fits the
  formatter's line limit.
- For a function with an unused parameter, dont cast it to void. Instead, use the attribute `[[maybe_unused]]`.
- Throw a project error class for an application failure. Do not throw
  `std::runtime_error`, `std::invalid_argument`, or a library exception directly.
