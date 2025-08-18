# Shrew 

A very mini lisp interpreter in C. I'm learning C, and I want to see how well 
I can write a lisp interpreter in it with minimal dependencies or use of the 
internet.  This will not be a usuable language in any real sense, but I hear 
it's always good to write a lisp when learning a new language.

### TODO 

- [x] Implement token
- [x] Implement lexer
- [x] Implement parser 
    - [x] Implement parsing of atoms 
    - [x] Implement parsing of lists
- [x] Implement hashtable
- [ ] Implement evaluator 
    - [x] Implement symbol interning 
    - [x] Implement lvals 
    - [x] Implement env 
    - [x] Implement atom evaluation 
    - [x] Implement list evaluation 
    - [ ] Implement special forms
        - [x] quote family
            - [x] quote
            - [ ] quasiquote
            - [ ] unquote
            - [ ] unquote-splicing
        - [ ] if
        - [ ] define
        - [ ] lambda
        - [ ] lambda
        - [ ] begin
        - [ ] cond
        - [ ] defmacro 
    - [ ] Implement builtins
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
        - [ ] Type predicates
            - [x] atom?
            - [x] list?
            - [x] null?
            - [x] number?
            - [x] symbol?
            - [x] string?
            - [x] list?
            - [x] pair?
            - [x] function?
        - [ ] Strings and conversion 
            - [ ] string-length
            - [ ] string-append
            - [ ] number->string
            - [ ] string->number
            - [ ] number->string
            - [ ] symbol->string
            - [ ] string->symbol
        - [ ] Functional 
            - [ ] apply
            - [ ] error
            - [ ] print
            - [ ] newline
            - [ ] map
            - [ ] filter
            - [ ] reduce
            - [ ] fold
            - [ ] gensym
            - [ ] eval
            - [ ] load

- [ ] Implement CLI tool
    - [ ] Repl 
    - [ ] File execution 

- [ ] Decide on memory management strategy, 
    - free all tokens, lexer, etc at the end
    - free parser and have it free lexer and have it free tokens
