# lamb-cpp

A C++ implementation of [lamb](https://github.com/tsoding/lamb) — a tiny pure functional programming language based on untyped lambda calculus with Normal Order reduction.

This is a simple learning project inspired by [lamb by tsoding](https://github.com/tsoding/lamb).

## Quick Start

```bash
g++ -o lamb lamb.cpp
./lamb
```

## Syntax

### Lambda Functions
```
\x.x
```

### Applications
```
(\x.x) 42
```

### Bindings (TODO)
```
id = \x.x
id 42  ; => 42
```
