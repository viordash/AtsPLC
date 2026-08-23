# Claude Code Instructions

## General

- Start each new conversation by reading `claude-context/context-claude.md` to understand the project context
- For deeper context on specific areas, read the relevant file from `claude-context/` directory
- User-facing documentation lives in `Docs/`; `README.md` is the entry point

## Naming Conventions

- Variables: `snake_case`
- Functions: `CamelCase`
- Do NOT use trailing underscore to mark member variables (e.g., use `secret` not `secret_`)

## C++ Style

- Use `struct` instead of `class`
- Member order: `protected` → `public` (from top to bottom)
- Prefer `protected` over `private` (easier for testing)
- Do NOT implement methods in header files, even small ones — all implementations go in `.cpp` files

## Build

```bash
make web      # Angular SPA, once after frontend changes
make app      # firmware
make tests    # unit tests
```

Or directly:

```bash
cd PLC_esp8266 && make -j $(nproc) size
cd Tests_esp8266 && make -j $(nproc)
```

## Angular

- Components must use separate files for template and styles:
  - `component-name.component.ts` - component class
  - `component-name.component.html` - template
  - `component-name.component.scss` - styles
- Use `templateUrl` and `styleUrl` instead of inline `template` and `styles`

## Tests

- Source files from the project that need to be included in tests should be added via `#include` in `tests/sources.cpp`
- NEVER TRY TO REPAIR TESTS THAT REQUIRE root
- Do NOT call `mock().checkExpectations()` manually — it is called automatically for all tests
- If a code module (class, struct, or functions) is in a separate file, its tests must be in a separate test file (e.g., `base32.cpp` → `base32_tests.cpp`, `totp.cpp` → `totp_tests.cpp`)
- All tests in one test file must belong to a single TEST_GROUP — do NOT mix multiple groups in one file

## Workflow

- Do NOT run `git commit` automatically — only suggest commit message text
- User will commit manually

## Commit Messages

- Keep commit messages short and concise
- Use conventional commits format: `type: short description`
- Types: `build`, `feat`, `fix`, `refactor`, `test`, `docs`, `chore`, `tooling`
- Do NOT add `Co-Authored-By` line
- Language: English

Example:
```
refactor: use function pointer for TAP device event handling
```

## Pull Request Format

- Language: English
- Format: Markdown

### Structure

```markdown
## <Stage Name>: <Short Description>

<Introductory paragraph: context from previous stage, what is implemented now, key achievement>

### Main changes

#### 1. <Change Category>

<Description of changes>

**Key files**: `path/to/file.cpp`, `path/to/file.h`

<Code examples where appropriate>

#### 2. <Next Category>

...

#### N. Tests

<Description of added tests>
```

### Guidelines

- Start with context: what was done in previous PR, what this PR adds
- Group changes by logical categories (architecture, optimization, refactoring, tests, docs)
- Mention specific files and classes affected
- Include code snippets for API changes or new patterns
- Use bullet points for lists of changes
- Number main sections (1, 2, 3...)
- Explain WHY, not just WHAT (e.g., "eliminates atomic check in hot path" instead of just "replaced virtual method")
