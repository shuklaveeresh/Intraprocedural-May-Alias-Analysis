🎯 Project Title
Intra-procedural May-Alias Analysis for LLVM IR

🛠️ Technology Stack
Compiler Infrastructure: LLVM

Language: C++

Development Tools: Git, GDB (for debugging)

📝 Project Description
This project involved the design and implementation of an intra-procedural, flow-sensitive, May-Alias analysis pass for LLVM. The core objective was to determine whether any two pointer variables within a single function might refer to the same memory location at a given program point. This type of static analysis is a fundamental component of modern compilers, enabling optimizations such as redundancy elimination and loop invariant code motion.

The analysis was built as a custom LLVM pass that traverses the control-flow graph (CFG) of a function, collecting and propagating points-to information. I implemented a data-flow analysis framework based on Kildall's algorithm, using a bitvector-based or set-based approach to represent the points-to sets for each pointer variable. The analysis accounts for various pointer operations, including:

Assignments: p = q (transferring points-to sets).

Address-of: p = &x (creating a new points-to entry).

Loads/Stores: p = *q and *p = q.

The pass operates on a program's LLVM Intermediate Representation (IR), which provides a standardized and abstracted view of the code. The final output is a structured report listing all potential aliasing relationships between pointer pairs at the end of each function.

🔍 Key Features
Flow-Sensitive: The analysis considers the specific order of statements, providing more precise results than a flow-insensitive analysis.

Intra-procedural: The analysis is scoped to a single function, without considering inter-procedural effects.

May-Alias: It provides a conservative "might alias" result, which is crucial for safety-critical optimizations.

Robustness: The implementation handles common C constructs like conditional branches (if/else) and loops by merging incoming data-flow information at join points (e.g., PHI nodes).

Automated Testing: I developed a set of private test cases with known alias relationships to validate the correctness of the pass.

💡 Project Outcomes
Successfully implemented a custom LLVM pass from scratch.

Gained a deep understanding of data-flow analysis algorithms, particularly Kildall's method.

Developed proficiency in working with the LLVM framework, including its IR and API.

Demonstrated an ability to apply theoretical program analysis concepts to a practical compiler engineering problem.

The final analysis pass correctly identified complex aliasing scenarios, including those influenced by control flow.








