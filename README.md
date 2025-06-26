# Mini Database Management System with Linked Data

This project implements a simple persistent database management system in C, supporting indexed and non-indexed record storage, parent-child (linked) data, and basic CRUD operations. The system uses binary files for data storage and a binary search tree (BST) for fast indexed access.

## Features

- **Indexed Storage:** Fast lookup, insertion, and deletion using a BST-based index.
- **Linked Data Support:** Supports parent-child relationships (e.g., Books and Authors) with linking and retrieval.
- **CRUD Operations:** Add, search (by index and non-index), and delete records.
- **Persistent Storage:** Data, index, and link files are stored on disk.
- **Backward Compatibility:** Handles cases where the linked repository is not used.
- **Test Automation:** Includes a test harness for automated validation.

## File Structure

- `roll_number_pds.c` – Core PDS (Persistent Data Store) implementation.
- `roll_number_book.c` – Book entity logic and repository interface.
- `roll_number_author.c` – Author entity logic and repository interface.
- `bst.c`, `bst.h` – Binary search tree implementation for indexing.
- `pds_sample.h` – Common definitions, error codes, and structures.
- `book.h`, `author.h` – Entity structure definitions.
- `pds_tester.c` – Automated test runner for validating functionality.
- `linked_data_demo.c` – Interactive demo for linked data operations.
- `testcase.in` – Test cases for automated testing.
- `README.txt` – Additional usage notes.

## Compilation

To compile the project, run:

```sh
gcc roll_number_book.c roll_number_author.c roll_number_pds.c bst.c pds_tester.c -o pds_test
```

To run the automated tests:

```sh
./pds_test testcase.in
```

## Usage

- **Automated Testing:**  
  The test runner reads commands from `testcase.in` and validates the implementation.
- **Interactive Demo:**  
  Compile and run `linked_data_demo.c` for a menu-driven demonstration of linked data features.

## Key Functionalities

- **CREATE**: Create a new repository and (optionally) a linked repository.
- **OPEN**: Open an existing repository.
- **STORE / STORE_LINKED**: Add records to the main or linked repository.
- **NDX_SEARCH / NON_NDX_SEARCH**: Search records by indexed or non-indexed fields.
- **NDX_DELETE**: Delete records by key.
- **LINK**: Link parent and child records.
- **SEARCH_LINKED / SEARCH_LINKED_BY_PARENT**: Retrieve linked records.
- **CLOSE**: Close the repository and persist the index.

## Notes

- The parent entity is **Book** and the child entity is **Author**.
- The system supports linking books to authors and retrieving all authors for a given book.
- All data is stored in binary files (`.dat`, `.ndx`), which are ignored by git via `.gitignore`.
