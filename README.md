
# libmeval

A easy-to-use and light-weight mathematical expression evaluator library written in C.

## Building from source

### Required programs

- make
- gcc (or clang)
- sh  (for generating the documentation pages, both man and HTML)

### Build instructions

#### Static library (and REPL)

- `make`

Library placed in the `lib/` directory.

#### Dynamic library (and REPL)

- `make shared-local`

Library placed in the `lib/` directory.

#### Building on Windows  (requires gcc, or clang, and GNUmake through MinGW)

You would have to define `MEVAL_REALLOCARRAY( ... )` as `realloc( ... )` due to Windows not having a `reallocarray( ... )` function that the library would attempt to use by default.

*NOT* any `install` targets will not work.
*NOT* any `shared` targets may not work.

#### Building the REPL

- `make repl`  Debug build of the repl.
- `make repl-rel`  Release version of the repl.
- `make repl-rel-static`  Statically linked, release version of the repl.

Built REPL's are placed within the `bin/` directory.

### Install (Root privilages required)

- `make install`  Installs the dynamic library to `/usr/local/`.
- `make shared-install`  Installs the dynamic library to `/usr/local/` and installs the `meval` binary to `/usr/local/bin/`.
