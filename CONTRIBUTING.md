# Contributing

Read [AGENTS.md](AGENTS.md), [the architecture](docs/ARCHITECTURE.md) and the
[current task list](.github/tasks/todo.md) before changing behavior.

Keep changes focused and use English for source, comments, commits and public
documentation. Follow the native C++20/Win32/COM design. Contributions to this
repository are provided under its [MIT license](LICENSE).

For code changes, use a focused branch and a pull request. Explain the concrete
problem, resulting behavior, checks completed and remaining limitations. Preserve
existing copyright and third-party notices. Declare material AI assistance with
the [source template](templates/source-header.cpp.txt) and a specific PR note;
do not claim human review that did not happen.

For owner-authorized documentation-only changes, commit and push on the current
branch with `[skip ci]`. No pull request, CI, build or test is needed. The native
workflow skips changes limited to Markdown, `docs/` and `LICENSE`. The owner's
administrator bypass allows direct documentation delivery to `main`.

Keep the changelog and task records aligned with the final change. An implemented
item moves from `todo.md` to `done.md`, then to `archive.md` after delivery. Keep
unverified behavior as open work, with an observable completion condition.

[Build commands](docs/BUILD.md) compile without registering the extension. Native
Explorer integration must be checked separately in an explicitly chosen test
environment. Report the Windows build and architecture when recording results.
