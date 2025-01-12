# Clock-Syntax-Highlighter
---
## Overview
Clock-Syntax is a Visual Studio Code extension providing syntax highlighting for the Clock Programming Language.

## Demo Screenshots
![image](https://github.com/user-attachments/assets/4cd2ca21-ddde-4b1a-9721-dcaa5fa06245)



## 🌟 Features
## Syntax Highlighting Capabilities

### Keywords Support:

if

else

while

for

return
---
### Declaration Types:

make

list

const
---
### Supported Literals:

Decimal numbers

Binary numbers (0b)

Hexadecimal numbers (0x)

---
## Code Example

```clk
// Demonstration of Clock Language Features
make age = 25;                   // Decimal declaration
make binary_value = 0b1010;      // Binary literal
make hex_color = 0x1A3F;         // Hexadecimal value

function calculateSum(a, b) {
    return a + b;
}

results = calculateSum(age,binary_value);
write(results);
```
---

## 🛠 Technical Specifications
Supported VS Code Versions
Minimum: ^1.96.0
File Extensions
.clk

---

## 🔍 Highlighting Details

Function name recognition

Escape sequence handling

Single and multi-line comment support

Comprehensive punctuation tracking

---

## 🚧 Current Limitations

No advanced error checking

Preliminary syntax support

---

# 📦 Installation
Open VS Code
Navigate to Extensions
Search "Clock-Syntax"
Click Install

# 🤝 Contributing
Open to community contributions

Report issues on GitHub

Pull requests welcome

# 📌 Future Roadmap

 Advanced error detection
 
 Linting support
 
 Improved identifier recognition
 
 Enhanced autocomplete
