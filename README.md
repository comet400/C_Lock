# C Lock Programming Language

**C Lock** (short for **C Language with LOCKED code**) is a programming language designed and built from scratch in C that will run and interpret itself using encrypted files, 
without relying on external frameworks (for the language, the encryption will be handled by standard OPENSSL AES-256 algorithms.
It is optimized for size, simplicity, and ease of understanding, making it an excellent tool for developers who want a lightweight, 
secure, and highly customizable language. (it is a work in progress at the moment).

## 🌟 Features

- **Built from Scratch**: No frameworks, just pure C, and standard libraries.
- **Simple Syntax**: Focused on being syntax friendly while maintaining power for advanced users.
- **Lightweight**: Designed to be small, features to make it faster like bytecode generation and JIT compilations is a work in progress.
- **Secure Execution**: Includes features to enforce safe and predictable program behavior, with error warnings.
- **Custom Interpreter**: Built specifically for C Lock, enabling seamless language interpretation and execution. (will be refactored to be turned into a compiler).

## 📖 Why C Lock?

C Lock was born from a desire of hiding source code, making it harder for attackers while also maintaining an easy program collaboration despite security features.

1. Simplify the encryption processes, also customizing the language itself with it.
2. Offer a secure and controlled environment for experimenting with low-level concepts. (enviroment will be hard coded for MVP).
3. Inspire and empower developers to create their own tools and languages, while ensuring their code privacy.

## 🔧 How It Works

### Interpreter
The C Lock compiler (`clock`) converts `.clk` source files into executable code using a custom-built architecture. It features:
- **Lexical Analysis**: Tokenizing the input code.
- **Parsing**: Building an Abstract Syntax Tree (AST).
- **Code Generation**: Interpreting the AST into readable instructions.
- **Built in terminal**: It is capable of encryption of any kinds of files easily, even for beginners.

### Code Example
Here’s a quick example of what writing in C Lock might look like:

```cl
//function listing() {
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
write("if its not 10 we go here");
stop;
}

return "this is the master return"; // master return
end
```

# However the interpreter would only compile and understand the code like this

``` cl
   ]~  * E   oQvT@  R}ԏ  T e 	 M  o5    z   y  j `\  Y / O I    [ ; Y]}@   d  ,  u!   °ݙ)   > X~ KYK   w ~^m    Ʒ  (0#Gg  o T  } єX e  P w jt '] pyW Y   {m	   {  f # ; u 'AzVYK9 $cN    <F   ;  o`i  V> .7     p    P[˖  8~[  Og<  c ١ SDL3s  k   ˢ dA   J  P   O I r ^/ 7  }v  M Z    ?t 0 ! >vg ivˈ
 L8  4 ay- w /s' L Ćɵ   p:G   6k)   e2: F) l_  #    ^H} =  b!64AɈ
FI 4aWM   qP > E  :;k   u/C  s >) [ d %24܄v2 5 \  9 v!
 A 7]LnH i r   Vߘ  wnR. wDF   %i)    d7n 2;{ 6 
 ~ !  (-' ' F b  z M$e  YQ  mE     q 1 Z'QhN  iRB
 8   {   I   a mT   ] $    FA  K 3 J  ^e   Hq ^ it& U g8naU      G  P jd q_  q  :a׆XVN@  ǞJ
¦   Eq B g}{ W Wd & ,\ 5 ~  V ?       S  
rj> R   2 ǲhM D   6G JS]  ܊W T S 0B l^q <   L   c Rs F  X Qm v  e y *eSm4_
:( p
hGM/ : pe;  >  S< jи  E >? 'Ox  h' Ÿc} z
ex҄B[񐹃 yp&7 #4 ^

```

