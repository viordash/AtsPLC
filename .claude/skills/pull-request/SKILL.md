---
name: pull-request
description: Compose a GitHub pull request description in English for the current branch relative to its parent branch, and optionally create the PR with gh. Use when the user asks to prepare, write or open a pull request.
---

# Skill: pull-request

Generate a GitHub pull request description for the current branch relative to the branch it was forked from, then create the PR on GitHub after explicit confirmation.

## Steps

1. Determine the parent branch:
   - Get the current branch name: `git rev-parse --abbrev-ref HEAD`.
   - Find the branch the current one was forked from by running:
     ```
     git show-branch -a 2>/dev/null \
       | grep '\*' \
       | grep -v "$(git rev-parse --abbrev-ref HEAD)" \
       | head -1 \
       | sed 's/.*\[\(.*\)\].*/\1/' \
       | sed 's/[\^~].*//'
     ```
   - If that produces no result, fall back to listing remote-tracking branches and picking the one whose tip is the closest ancestor of HEAD:
     ```
     git branch -r --sort=committerdate \
       | grep -v "$(git rev-parse --abbrev-ref HEAD)" \
       | while read b; do git merge-base --is-ancestor "$b" HEAD 2>/dev/null && echo "$b"; done \
       | tail -1
     ```
   - If neither yields a result, use `main`.
   - Use the identified parent branch name as `<parent>`.
   - Find the common ancestor commit: `git merge-base HEAD <parent>`.
2. Collect the change set:
   - Run `git log --oneline <merge-base>..HEAD` to list commits on this branch.
   - Run `git diff <merge-base>..HEAD --name-only` to list all touched files.
   - Run `git diff <merge-base>..HEAD` for the full diff (use for analysis, not verbatim output).
   - Clearly state which parent branch was detected at the top of your output (e.g. "Parent branch: main").
3. Analyze the changes:
   - Identify the **stage name**: the high-level theme of this PR (e.g. "Operating modes", "Web interface", "Ladder editor").
   - Group changes into logical categories (architecture, optimization, refactoring, tests, docs, build).
   - For each category note the **why**, not just the what.
   - If the change set is scoped to one platform or to the Web interface, say so (`ESP32`, `ESP8266`, `Linux`, `WEB`).
4. Compose the PR description in **English** using the format below and print it.
5. Ask the user whether to create the PR on GitHub. Only after an explicit yes:
   - Push the branch if it has no upstream: `git push -u origin <current-branch>`.
   - Write the body to the scratchpad directory and run:
     ```
     gh pr create --base <parent> --head <current-branch> --title "<title>" --body-file <path>
     ```
   - Print the resulting PR URL.
   - If a PR for this branch already exists, update it instead: `gh pr edit --body-file <path>`.

## Output format

```markdown
## <Stage Name>: <Short Description>

<Introductory paragraph: context from the parent branch / previous stage, what is implemented in this PR, key achievement>

### Main changes

#### 1. <Change Category>

<Description of changes - explain WHY, not just WHAT>

**Key files**: `path/to/file.cpp`, `path/to/file.h`

#### 2. <Next Category>

...

#### N. Tests

<Description of added or updated tests>
```

The PR title is `<Stage Name>: <Short Description>` - the same text as the top-level heading, without the leading `## `.

### Rules

- Language: **English** throughout (headings, body, everything).
- Use simple English that non-native speakers read easily:
  - common words, no idioms or slang, avoid phrasal verbs when a plain verb exists ("remove", not "get rid of");
  - short sentences, one idea per sentence, active voice.
- Keep it short: a reader should get the point in a minute.
  - Intro: 2-4 sentences.
  - Each section: 2-5 bullets, one line each where possible.
  - Leave out details the diff already shows (helper names, mock setup, commit hashes), unless they explain a behavior change.
- Lead with context: what existed before, what this PR adds, what the key achievement is.
- Number the main sections (1, 2, 3 ...).
- Mention specific files and classes affected under each section.
- Include short code snippets only when they illustrate an API change or a new pattern.
- Use bullet points inside sections for lists of changes.
- Do **not** use Markdown tables - they render poorly on GitHub. Represent tabular data as plain text with spaces, wrapped in a fenced code block:
  ````
  ```
  Column A    Column B    Column C
  value1      value2      value3
  ```
  ````
- Do **not** include a test plan checklist or AI attribution line.
- Do **not** run `git commit`, `git push` or any `gh` command before the user has confirmed step 5.

## Output

Print the proposed PR description inside a fenced markdown code block, then ask whether to create the PR.
