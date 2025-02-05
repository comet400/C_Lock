# C Lock Programming Language Syntax Guide

**C Lock** combines the simplicity of Python with the structure of C, offering easy syntax for developers. This guide provides an overview of the main syntax features and usage in C Lock.

---

## Syntax Overview

C Lock introduces built in tools for encryption and decryption, the user may use any interpreter that is adequate for the task. Here's a breakdown of the syntax features:

---

### Variable Initialization

- **Normal Variable Initialization**:
  Use the `=` operator for simple assignments.
  ```cl
  variable1 = 10;
  variable2 = "string";
  variable3 = 34.5;
  variable4 = true;
  nullVariable; //initializes it to null if no assigment is provided
  make variable5 = 0b0101101; // binary and hex are converted to int
  make variable6 = 0x112f; // safe variable declaration
  ```

  Safe Variable Initialization: Use the make keyword for explicitly declaring variables, ensuring cleaner and safer code. if make is used without an assigment "=" an error will be received.

---
- **Arrays and lists Initialization**:
  ```cl
  list thisList = { 1,2.7,"hello",true, 0b01101 }; \\ each element is treated differently allowing the user to put different datatypes into it
  ```

  **Loops**:
  While Loops: Standard while loop structure for repetitive tasks.
  ```cl
  make i = 0;
  while (i < 10) {
    write(i);
    i += 1;
  }
  ```

  For Loops: Use the for (start to end) structure for fixed iterations. The loop does not increment the variable inside automatically.
  ```cl
  for (0 to 100) {
    write("This will run 100 times!"); \\custom for loop
  }
  ```

---
  **Conditional Statements**:
  ```cl
  if (x == 10) {
    write("x is 10!");
  }

  switch (x) {
  when 10:
    write("yes we have switch too");
    stop;
  default:
    write("if it's not 10 we go here");
    stop;
  }
  ```

  stop statements also work in order to stop a loop

---
  **Functions**:
  Define functions with the function keyword.
  ```cl
  //very simplistic TwoSum resolution using Clock
  
  function listing() {
    list shop = {2, 7, 11, 15, 20, 35, 40, 50, 65, 80};
    make target = 100;

    make i = 0;
    while (i < 10) {
        make j = i + 1;
        while (j < 10) {
            if (shop[i] + shop[j] == target) {
                write(shop[i]);
                write(shop[j]);
                return "it works see?";
            }
            j += 1;
        }
        i += 1;
    }
  }
  
  variable1 = listing(); //function call assigning return values to variable1
  write(variable1);
  ```

---
  **Returning and Stopping Execution**:
  ```cl
  return "program exit"; 
  ```
  returns will stop the execution of a function, however, if used outside of any function it will stop the execution of the program and only prints its value if called.
  Example: Clock returned: "program exit";

  Use stop to exit a specific case or block of code or stop loops;
  ```cl
  when 10:
    write("Stopping here.");
    stop;
  ```

---
  ##Example Program
  ```cl
  function listing() {
    list shop = {2, 7, 11, 15, 20, 35, 40, 50, 65, 80};
    make target = 100;
    make i = 0;
    make j = 0;

    while (i < 10) {
        make j = i + 1; // Start j from i + 1 to avoid duplicate pairs
        while (j < 10) {
            if (shop[i] + shop[j] == target) {
                write(shop[i]);
                write(shop[j]);
                return "it works see?"; // Stop both loops by returning from the function
            }
            j += 1;
        }
        i += 1;
    }
  }

  lukas = listing(); // normal variable init
  write(lukas);
  
  make x = 10; // safe variable init version
  switch (x) {
  when 10:
    write("yes we have switch too");
    stop;
  default:
    write("if it's not 10 we go here");
    stop;
  }

  return "master return for Clock"; // master return

  for (0 to 5) { 
    write("This loop runs 5 times.");
  }
  ```

##Current Built-In functions
```cl
// Print and Input
write("Welcome to C Lock!");
name = input("Enter your name: ");
write(name);


// Date and Time
date = date_time();
time = current_time();
timestamps = timestamp();
write(date);
write(time);
write(timestamps);


// File Handling
write("Creating a new file...");
write_file("example.txt", "Hello, C Lock!");
write("File created and content written.");

write("Appending content to the file...");
append_file("example.txt", "Appended text line");

write("Reading file content...");
content = read_file("example.txt");
write("File content:\n", content);

write("Checking if file exists...");
exists = file_exists("example.txt");
write(exists);

write("Getting file size...");
size = file_size("example.txt");
write(size);

write("Listing files in current directory...");
list_files(".");

write("Deleting the file...");
delete_file("example.txt");
write("File deleted successfully.");

// Wrap-up
write("That's all for the C Lock built-in functions demo!");

return "more functions in the future";
```

---
Feel free to contribute to or explore the C Lock Programming Language and help us make it even better!

  

  


  
