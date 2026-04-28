# lamb-cpp

A tiny, pure functional programming language based on untyped lambda calculus. Built from scratch in C++20 as a learning exercise, inspired by [tsoding/lamb](https://github.com/tsoding/lamb).

## Features

- **Pure Lambda Calculus**: Minimalist syntax with core support for abstractions, applications, and bindings.
- **Normal Order Reduction**: Evaluates the leftmost, outermost redexes first.
- **Hygienic Substitution**: Safely handles variable shadowing and prevents variable capture during AST reduction.
- **Interactive REPL**: A built-in loop for defining combinations and evaluating expressions.

## Quick Start

```bash
$ g++ -std=c++20 lamb.cpp -o lamb
$ ./lamb
```

For a better CLI experience with history and line-editing capabilities, wrapping it with [rlwrap](https://github.com/hanslub42/rlwrap) is highly recommended:

```bash
$ rlwrap ./lamb
```

Add `-d` flag to enable debug mode, allowing you to walk through the AST reduction step-by-step by pressing `Enter`:

```bash
$ rlwrap ./lamb -d
> (\f.\x.f x) fun var
((\x.fun x) var)
(fun var)
> 
```

Run a script file or preload an environment file with a file path.

```bash
$ ./lamb factorial.lamb
(\f.(\x.f (f (f (f (f (f x)))))))
$ ./lamb -e factorial.env
Welcome to Lamb (C++ Edition)
Type 'exit' or 'quit' to exit.
> FACT 3
(\f.(\x.f (f (f (f (f (f x)))))))
>
```



## Syntax

### Lambda Functions
```
> \x.x
```

### Applications
```
> (\x.x) y
```

### Pairs

```
> [x x]
> [a [b [c d]]]
```

### Bindings

```
> define TRUE \t.\f.t
```

You can build complex logic entirely out of functions. Here is a complete environment setup to calculate factorials using the Y Combinator:

```bash
> define Y_C (\f.(\x.f(x x)) (\x.f(x x)))
Defined: Y_C
> define TRUE \t.\f.t
Defined: TRUE
> define FALSE \t.\f.f
Defined: FALSE
> define 0 \f.\x.x
Defined: 0
> define SUCC \n.\f.\x.f (n f x)
Defined: SUCC
> define 1 SUCC 0
Defined: 1
> define 2 SUCC 1
Defined: 2
> define 3 SUCC 2
Defined: 3
> define ISZERO \n.n (\x.FALSE) TRUE
Defined: ISZERO
> define MULT \m.\n.\f.m (n f)
Defined: MULT
> define PRED \n.\f.\x.n (\g.\h.h (g f)) (\u.x) (\u.u)
Defined: PRED
> define FACT_STEP \f.\n. ISZERO n 1 (MULT n (f (PRED n)))
Defined: FACT_STEP
> define FACT Y_C FACT_STEP
Defined: FACT
```

You can learn more about lambda calculus on: [A Tutorial Introduction to the Lambda Calculus](http://www.inf.fu-berlin.de/lehre/WS03/alpi/lambda.pdf) 

## Formal Grammar (BNF)

``` 
<expr> ::= <term> { <term> }

<term> ::= <name>
         | "\" <name> "." <expr>
         | "(" <expr> ")"
         | "[" <term> <term> "]"
```
