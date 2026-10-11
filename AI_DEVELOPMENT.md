# AI development disclosure

- **Project:** MyComputerOnSteroid.
- **Maintainer:** Umberto Giacobbi, https://umbertogiacobbi.biz.
- **Scope:** initial repository and native COM provider bootstrap, 2026-10-11.
- **AI tool:** OpenAI Codex. No exact deployed model identifier is recorded.

## Human direction

I specified a native C++ launcher in Explorer with separate groups for drives,
folders, favorite apps and files. I chose MIT, supplied my existing repositories
as convention references, and clarified that this project participates in the
VibeWare movement while I remain its creator and publisher.

## AI contribution

Codex researched the public Shell APIs, adapted repository conventions, wrote
the initial C++/COM provider, CMake configuration, build automation and repository
documentation. It also prepared publication and recorded validation results.

The instructions and intended architecture were human-directed. A human line-by-line
code review, acceptance of the running UI and a security review have not been
recorded for this bootstrap. AI-assisted inspection is not a substitute for them.

## Evidence and limitations

See [validation](docs/VALIDATION.md), Git history and
[GitHub Actions](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions).
The empty provider does not validate the intended launcher, modern context menus,
favorite deletion policy or background rendering. Keep this disclosure current
as those features are implemented and checked.

[Read the VibeWare manifesto](https://umbertogiacobbi.biz/vibeware/manifesto).
