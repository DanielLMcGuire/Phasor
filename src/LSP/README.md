Each file is represented as a **DocumentState**, which stores the raw text, an AST, a table of top-level symbols, and any parsing errors. When a file is opened or edited, the LSP server host will:

1. **Parse the code** using the lexer and parser.
2. **Build a symbol table** of all top-level declarations (functions, structs, variables, exports).
3. **Tracks line offsets** for converting between editor positions and parser positions.
4. **Generates diagnostics** for syntax or parsing errors.

For editor features:

* **Hover** shows the signature of a function, struct, or variable under the cursor.
* **Go-to-definition** finds where a symbol was declared.
