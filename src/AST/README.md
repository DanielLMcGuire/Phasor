## Core Components

### 1. `Node`, `Expression`, `Statement`
* **`Node`**: Base node. Includes virtual `print` method for debugging and tracks `line` and `column`.
* **`Expression`**: Derived from `Node`, representing segments of code that resolve to a value (e.g., numbers, math, function calls).
* **`Statement`**: Derived from `Node`, representing actions or commands that do not necessarily return a value (e.g., loops, variable declarations).

### 2. Expressions
* **Literals**: `NumberExpr`, `StringExpr`, `BooleanExpr`, and `NullExpr`.
* **Operators**:
    * **Unary**: negation, logical NOT, and pointer operations (`&` address-of and `*` dereference)
    * **Binary**: arithmetic (`+`, `-`, `*`, `/`, `%`), logical (`&&`, `||`), and comparisons (`==`, `<`, `>=`, etc.).
    * **Postfix**: increment (`++`) and decrement (`--`).
* **Complex Access**: `ArrayAccessExpr`, `MemberAccessExpr` (using `.`), and `CallExpr'.

### 3. Statements
* **Variable & Types**: Includes `VarDecl` for variables and `TypeNode` to represent types, including support for pointers and multi-dimensional arrays.
* **Control Flow**: Standard programming constructs like `IfStmt`, `WhileStmt`, `ForStmt`, `SwitchStmt` (with `CaseClause`), `BreakStmt`, and `ContinueStmt`.
* **Organization**: `BlockStmt` for grouping code, `ImportStmt`/`ExportStmt` for modularity, and `FunctionDecl` for defining reusable logic.
* **?**: `UnsafeBlockStmt`, reserved for future use.

### 4. Data
* **`StructDecl` & `StructField`**
* **`StructInstanceExpr`**