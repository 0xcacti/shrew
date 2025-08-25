# Shrew 

A very mini lisp interpreter in C. 

I'm learning C, and a friend told me it's always good to build a lisp 
in a new language you are learning, so I took that advice to heart.  
I wanted to build it entirely without the help of internet or AI, but 
it turns out I don't know how to build a lisp interpreter as well as I 
thought I did in the beginning, so I needed to do a fair bit of research. 
Nonetheless, it was an extremely useful project for me to learn about 
good ways to manually manage memory.  

All that said, it's done.  The language is fully implemented with a lexer, parser, 
and evaluator.  It supports closures, macros, a boatload of builtins, several 
special forms, and garbage collection. 

Below is my list of todos.  Maybe it will give a sense of everything involved 
in building the project.  

### TODO 

- [x] Implement token
- [x] Implement lexer
- [x] Implement parser 
    - [x] Implement parsing of atoms 
    - [x] Implement parsing of lists
- [x] Implement hashtable
- [x] Implement evaluator 
    - [x] Implement symbol interning 
    - [x] Implement lvals 
    - [x] Implement env 
    - [x] Implement atom evaluation 
    - [x] Implement list evaluation 
    - [x] Implement user defined functions
    - [x] Implement closures
    - [x] Implement evaluate multiple expressions
    - [x] Implement special forms
        - [x] define
        - [x] set
        - [x] lambda
        - [x] quote family
            - [x] quote
            - [x] quasiquote
            - [x] unquote
            - [x] unquote-splicing
        - [x] if
        - [x] cond
        - [x] begin
        - [x] defmacro 
    - [x] Implement builtins
        - [x] Arithmetic
            - [x] +
            - [x] -
            - [x] *
            - [x] /
            - [x] mod 
            - [x] abs
            - [x] min
            - [x] max
            - [x] floor
            - [x] ceil
            - [x] round
            - [x] trunc
            - [x] sqrt 
            - [x] exp
            - [x] log
        - [x] Comparison and equality
            - [x] =
            - [x] <
            - [x] <=
            - [x] >
            - [x] >=
            - [x] eq?     *(identity)*
            - [x] equal?  *(deep)*
        - [x] Booleans 
            - [x] not
            - [x] and 
            - [x] or
        - [x] Lists / pairs 
            - [x] cons 
            - [x] car
            - [x] cdr
            - [x] list
            - [x] length
            - [x] append
            - [x] reverse
        - [x] Type predicates
            - [x] atom?
            - [x] list?
            - [x] null?
            - [x] number?
            - [x] symbol?
            - [x] string?
            - [x] list?
            - [x] pair?
            - [x] function?
        - [x] Strings utils
            - [x] string-length
            - [x] string-append
        - [x] Casting
            - [x] number->string
            - [x] string->number
            - [x] number->string
            - [x] symbol->string
            - [x] string->symbol
        - [x] Functional 
            - [x] apply
            - [x] map
            - [x] reduce
            - [x] foldl
            - [x] foldr
            - [x] filter
            - [x] gensym
            - [x] error
            - [x] eval
            - [x] load
        - [x] I/O
            - [x] print
            - [x] newline
- [ ] Implement garbage collector
- [x] Implement CLI tool
    - [x] Repl 
    - [x] File execution 
