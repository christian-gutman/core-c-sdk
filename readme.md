# Core C SDK ⚙️
### Shared Subtree Component for Multi-Project Integration

This repository contains the **Core C SDK**, a centralized foundation shared across multiple independent projects. By leveraging **Git Subtree**, we ensure that all sibling projects share a single source of truth without the overhead or coupling of traditional submodules.

## 🌟 Key Architecture Benefits
* **Cross-Project Synchronization:** Updates made to the core can be easily pulled (`subtree pull`) or pushed back (`subtree push`) from any consuming project.
* **Independent Builds:** Consuming projects embed the core code directly into their repository, removing the need for internet access or package managers during compilation.
* **Clean CMake Linkage:** Seamlessly integrated into downstream build systems via CMake's `FetchContent` or local subdirectory mapping.

 This repo is Restrict pushes - Require a pull request before merging