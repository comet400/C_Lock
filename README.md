# C Lock Programming Language

**C Lock** (short for **C Language with LOCKED code**) is a secure programming language built from scratch in **C**. It combines **lightweight execution**, **encryption with AES-256**, and a mix of **Python-C-inspired syntax**, offering a customizable and secure coding experience. In the current world cybersecurity is a must, so this language is trying its best to be developed, and perform in way that makes revese engineering harder for future attacks.

Whether you want to explore **low-level language design**, **secure your scripts**, or learn about **interpreters**, C Lock is being designed to help developers while prioritizing **code privacy**.

---

## Features

- **Built from Scratch**: No frameworks—just pure C and standard libraries.
- **Secure by Design**: Uses AES-256 to ensure code privacy and prevent tampering. (Might change to quantum encryption for future treats)
- **Lightweight**: Optimized for simplicity and size, with plans for bytecode and AOT compilation. (compilation still under development)
- **Custom Interpreter**: Reads encrypted `.clk` files for secure execution (works with any kind of text file).
- **Python-C Syntax**: Combines Python's simplicity with C's power.

---

## Why Choose C Lock?

C Lock is designed for developers who need:
1. **Privacy**: Hide and encrypt source code to protect intellectual property.
2. **Experimentation**: A secure environment for exploring language design.
3. **Simplicity**: A beginner-friendly yet powerful language.

---

## How It Works

### Interpreter Workflow

The C Lock compiler (`clock`) converts `.clk` source files into secure, executable instructions:
1. **Lexical Analysis**: Tokenizes input code.
2. **Parsing**: Builds an Abstract Syntax Tree (AST).
3. **Code Generation**: Interprets the AST into instructions.
4. **Encryption**: Protects source code using AES-256 encryption.

---

## Quick Start

### Installation

1. Clone the repository:
```bash
git clone https://github.com/comet400/Clock.git
cd Clock
```


2. Build the intepreter
   
Linux
```bash
   make
```

Windows
```bash
     mingw32-make
```

or any other C compiler you might have.

After installation, you should be able to receive, an .exe file or .elf depending on your OS.

# Runtime and Terminal mode
If you simply open or double click the exe file, you will enter the terminal mode. This will allow you to write lines of code and enter them by pressing enter.
After you are completely done, in an empty line, you may use the command "END" in order to compile the code normally or "DEBUG" to see all AST nodes and tokens.
To use runtime mode, simply write a script yourself using Clock syntax, then, using the cllc (C Lock Language Compiler) command, put the parameter.

```bash
cllc test.clk
```

## Getting started
All the rules for the language and how it works are easily found in the documents README. If you want to know which built in functions are already implemented and how they work
you can easily find them in the documents.


## Use Cases
Proprietary Scripts: Secure your code with AES-256 encryption.
Learning Resource: Dive into interpreters and encryption.
Lightweight Applications: Use C Lock for rapid, secure development.

#Contributing
Contributions are welcome! Here's how you can help:

Check out the issues.
Fork the repository and submit a pull request.

### C Lock is in very early development and is not yet ready for any kind of commercial use
