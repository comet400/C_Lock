# Clock Interpreter is Here!

No experience with **C** or **Makefiles**? No problem! We've got you covered.

---

## About the Interpreter
This interpreter is designed for simplicity and efficiency. Whether you're here to test the language or dive deeper, the process is straightforward.

- **Lightweight**: At just **137KB**, it’s incredibly compact—your system won’t even notice it’s there!
- **Optimized**: Developed with hard optimization tools for maximum performance.

---

## Getting Started
You can run the Clock Interpreter easily with the provided executable file. Follow these steps:

1. Place the `.exe` file in the directory containing your code.
2. Use the following commands to run the interpreter:

   ```bash
   cllc [filename] \\RuntimeMode
   # Runs the specified file in runtime mode.

   cllc
   # Launches terminal mode for interactive use.
   ```
   **[WARNING]: The easy installation was tailored for Windows. If you're using Linux, we highly recommend using the provided Makefile for installation.**

# Runtime and Terminal mode
If you simply open or double click the exe file, you will enter the terminal mode. This will allow you to write lines of code and enter them by pressing enter.
After you are completely done, in an empty line, you may use the command "END" in order to compile the code normally or "DEBUG" to see all AST nodes and tokens.
To use runtime mode, simply write a script yourself using Clock syntax, then, using the cllc (C Lock Language Compiler) command, put the parameter.
