# Naming Conventions

This document outlines the naming conventions used in the Tessellation Project. The conventions have been established to ensure code readability, maintainability, and to facilitate easy understanding for new contributors.

## General Principles

- **Clarity and Descriptiveness**: Names should clearly reflect the purpose and nature of the variable, function, or class they represent.
- **Consistency**: Apply naming conventions consistently throughout the project.
- **Brevity**: While names should be descriptive, unnecessary length should be avoided to maintain readability.

## Variables

- **Local Variables and Parameters**: Use camelCase for local variables and function parameters.
  - Example: `localVariable`, `functionParameter`
- **Member Variables**: Use a trailing underscore (`_`) to distinguish member variables.
  - Example: `memberVariable_`
- **Constants and Enumerators**: Use UPPER_CASE with underscores for constants and enumerators.
  - Example: `CONSTANT_VALUE`

## Pointers

- **Raw Pointers**: Use a `p` prefix for raw pointers, but prefer smart pointers unless raw pointers are necessary for non-ownership scenarios.
  - Example: `pNode`
- **Unique Pointers**: Use `up` prefix or suffix for `std::unique_ptr`.
  - Example: `upNode`, `nodeUp`
- **Shared Pointers**: Use `sp` prefix or suffix for `std::shared_ptr`.
  - Example: `spNode`, `nodeSp`
- **Weak Pointers**: Use `wp` prefix or suffix for `std::weak_ptr`.
  - Example: `wpNode`, `nodeWp`

## Containers

- **Vectors**: Name vectors with a plural form of the contained element, indicating the type if it's a pointer, especially for `std::unique_ptr`.
  - Example: `nodes` for `std::vector<Node*>`, `upNodes` for `std::vector<std::unique_ptr<Node>>`

## Classes and Structs

- **Classes and Structs**: Use PascalCase for classes, structs, and enumeration types.
  - Example: `TessShape`, `SnapPair`

## Functions and Methods

- **Functions and Methods**: Use camelCase for function and method names. Begin with a verb to indicate action.
  - Example: `createNewTriangle`, `findClosestSnapPoints`

## File Names

- **Header and Source Files**: Use snake_case for file names.
  - Example: `tess_shape.h`, `tess.cpp`

## Type Aliases

- **Type Aliases**: Use PascalCase for type aliases to indicate they are types.
  - Example: `NodePtr`, `SnapPairVector`

## Comments and Documentation

- Ensure to comment complex logic or where the naming convention might not sufficiently convey the purpose or behavior.
- Use Markdown files (`*.md`) for broader project documentation, such as this naming convention guide.

This guide is intended to evolve as the project grows. Contributors are encouraged to discuss potential improvements to these conventions to better suit the project's needs.
