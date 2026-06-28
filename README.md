
*This branch, `master`, is a development branch, SEE the `2.x` branch (or similar) for stable branches*

# libmeval

An easy-to-use and light-weight mathematical expression evaluator library written in C.

## Building from source

### Required programs

- make
- gcc (or clang)
- bash (or sh)  (for generating the documentation pages, both man and HTML)
- pandoc   (for generating the documentation pages, both man and HTML)

### Build instructions

#### Static library (and REPL)

- `make`

Library placed in the `lib/` directory.

#### Dynamic library (and REPL)

- `make shared-local`

Library placed in the `lib/` directory.

#### Building on Windows  (requires gcc, or clang, and GNUmake through MinGW)

You would have to define `MEVAL_REALLOCARRAY( ... )` as `realloc( ... )` due to Windows not having a `reallocarray( ... )` function that the library would attempt to use by default. See [docs/libmeval.3.md](docs/libmeval.3.md) for defining the macro.

*NOTE* any `install` targets will not work.

*NOTE* any `shared` targets may not work.

#### Building the REPL

- `make repl`  Debug build of the repl.
- `make repl-rel`  Release version of the repl.
- `make repl-rel-static`  Statically linked, release version of the repl.

Built REPL's are placed within the `bin/` directory.

### Install (Root privilages required)

- `make install`  Installs the dynamic library to `/usr/local/`.
- `make shared-install`  Installs the dynamic library to `/usr/local/` and installs the `meval` binary to `/usr/local/bin/`.
