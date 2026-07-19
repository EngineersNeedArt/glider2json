# glider2json

Command-line tool that converts Glider 4.0 (classic Mac) house files into a
single JSON house file for the modern rewrite.

## Build

```
make
```

Produces a `glider2json` binary. Pure C11, no dependencies beyond libc and
the bundled `cJSON`. Should build as-is on macOS/Linux; on Windows use
WSL/MSYS2/MinGW (no POSIX-specific calls are used beyond `/`-style paths in
`baseNameOf`, which is cosmetic only — the `-o` path itself works fine
either way).

## Usage

```
glider2json [-o output] house_file [house_file ...]
```

- Pass one or more house files. If a house is split across multiple linked
  files (Glider houses can chain via `nextFile`/`firstFile`), pass all of
  them — they'll be stitched together in the correct order automatically,
  regardless of the order you list them in.
- `-o <path>` sets the output path (default: `house`). A `.house4`
  extension is always appended automatically if it isn't already there
  (case-insensitive check, so `-o Foo.HOUSE4` won't get doubled up).
- `-h` / `--help` prints usage.

## What changed from the original Xcode app

- `HouseConvert.c` / `HouseConvert.h` / `cJSON.c` / `cJSON.h` are unchanged —
  they were already pure C.
- `main.m` + `AppDelegate.m` (Cocoa `NSOpenPanel`/`NSSavePanel` UI) were
  replaced by `main.c`, which does plain `argv` parsing and `fopen`/`fread`/
  `fwrite` file I/O.
- One behavior fix: the original app `malloc`'d exactly `rawData.length`
  bytes for each house buffer, even though the code then reads/writes it
  as a full `HouseStruct`. For a short or malformed file that's an
  out-of-bounds read/write. `main.c` now always allocates a full,
  zero-filled `sizeof(HouseStruct)` buffer and warns (but still proceeds)
  if a file's size doesn't match what's expected.
- Everything else — struct layout, endianness fixing, Pascal-string
  conversion, the object/room JSON mapping, and the multi-file
  `firstFile`/`nextFile` ordering algorithm — is byte-for-byte the same
  logic as your original code.
