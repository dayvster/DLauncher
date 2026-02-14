# Contributing to DLauncher

Thank you for your interest in contributing!

## How to Contribute
1. Fork the repository and create your branch from `main`.
2. Make your changes and ensure the code builds and passes tests.
3. Use [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) for all commit messages and pull requests.
4. Submit a pull request with a clear description of your changes.
5. Address any review comments and update your PR as needed.

## Commit Message Guidelines

All contributions must use [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/). This helps automate changelogs and makes history easy to understand.

**Format:**
```
type(scope): short description

[optional body]
[optional footer(s)]
```

**Types:**
- feat: A new feature
- fix: A bug fix
- docs: Documentation only changes
- style: Changes that do not affect meaning (white-space, formatting)
- refactor: Code change that neither fixes a bug nor adds a feature
- perf: Performance improvement
- test: Adding or correcting tests
- chore: Maintenance

We also accept these concise aliases/types and encourage their use in commit messages:

- `bugfix:` (alias for `fix:`) — use when clarifying a bug-fix type
- `type:` — small project-specific type to indicate the change relates to "type" metadata

Preferred top-level types to use: `feat:`, `fix:`/`bugfix:`, `chore:`, `type:`.

**Examples:**
- `feat(launcher): add frequency-based sorting`
- `fix(theme): correct color parsing`
- `docs(roadmap): update future plans`

## Code Style
- Use clear, descriptive variable and function names.
- Remove non-doc comments before submitting.
- Prefer modern C++ (C++17 or later).
- Keep code modular and well-documented.

## Issues & Feature Requests
- Please open issues for bugs or feature requests.
- Tag issues appropriately (e.g., bug, enhancement, question).
- Include steps to reproduce or a clear description of the desired feature.

## Issue Naming & Tagging

- Use clear, descriptive titles for issues (e.g., "Launcher crashes on startup" or "Add dark mode theme").
- Tag issues with appropriate labels: `bug`, `feature`, `enhancement`, `question`, etc.
- For bugs, include steps to reproduce and expected/actual behavior.
- For features, describe the desired outcome and rationale.

## Communication
- Be respectful and constructive in all discussions.
- Collaborate and help others when possible.

---
Happy coding!
