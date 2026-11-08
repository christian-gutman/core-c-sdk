# Core C SDK ⚙️
### Shared Subtree Component for Multi-Project Integration

This repository contains the **Core C SDK**, a centralized foundation shared across multiple independent projects. By leveraging **Git Subtree**, we ensure that all sibling projects share a single source of truth without the overhead or coupling of traditional submodules.

## 🌟 Key Architecture Benefits
* **Cross-Project Synchronization:** Updates made to the core can be easily pulled (`subtree pull`) or pushed back (`subtree push`) from any consuming project.
* **Independent Builds:** Consuming projects embed the core code directly into their repository, removing the need for internet access or package managers during compilation.
* **Clean CMake Linkage:** Seamlessly integrated into downstream build systems via CMake's `FetchContent` or local subdirectory mapping.

## ⚠️ Read-Only Component Notice

The `external/core_c_sdk` directory is a **read-only component** in this repository. 
* **Do not modify** any files inside this folder locally.
* Any direct changes to this core will be rejected by the pre-commit hooks or overwritten during build setup.
* If you need to fix a bug or add a feature to the SDK, you must submit a Pull Request directly to the [Central Core C SDK Repository]([https://github.com](https://github.com/christian-gutman/core-c-sdk)).

 This repo is Restrict pushes - Require a pull request before merging
