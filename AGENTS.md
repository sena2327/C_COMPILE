# Repository Guidelines

## Project Structure & Module Organization
This is a small C11 compiler project. The repository is flat:
- Source code lives at the root: `main.c`, `parse.c`, `codegen.c`, and shared definitions in `9cc.h`.
- Tests are scripted in `test.sh`.
- Build configuration is in `Makefile`.

## Build, Test, and Development Commands
- `make`: builds the `9cc` compiler binary using the system `CC`.
- `make test`: builds `9cc` and runs `test.sh`.
- `make clean`: removes `9cc`, object files, and temp artifacts like `tmp*`.

Example single compile:
```sh
./9cc "1+2;" > tmp.s
x86_64-linux-gnu-gcc -static -o tmp tmp.s
qemu-x86_64 ./tmp
```

## Coding Style & Naming Conventions
- Language: C11 (see `CFLAGS` in `Makefile`).
- Indentation: spaces, no tabs; follow the existing file’s indentation depth.
- Braces: K&R style (`if (...) {`).
- Naming: functions and variables in `snake_case` (`tokenize`, `user_input`); types in `CamelCase` (`Node`, `Token`, `LVar`); enums in `ALL_CAPS`/`ND_*`, `TK_*`.
- No formatter is enforced; keep diffs minimal and consistent with nearby code.

## Testing Guidelines
- Tests are in `test.sh` and use `x86_64-linux-gnu-gcc` plus `qemu-x86_64`.
- Add new assertions using the existing `assert EXPECTED "input;"` pattern.
- Run `make test` before opening a PR.

## Commit & Pull Request Guidelines
- Commit messages are short, lowercase, and action-oriented (examples from history: `add new function`, `test`).
- PRs should include:
  - A brief summary of changes.
  - Tests run (e.g., `make test`) or a reason for skipping.
  - Linked issue/feature context if applicable.

## Architecture Overview
High-level flow: input is tokenized in `parse.c`, parsed into AST nodes declared in `9cc.h`, and compiled to assembly in `codegen.c`, with orchestration in `main.c`.
