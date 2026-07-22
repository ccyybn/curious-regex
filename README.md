## Curious Regex

A learning project exploring different ways to implement a regular expression matching engine.

The grammar of this regular expression is simplified, but still retains its most intriguing features: nested parentheses, branching, and repetition (for example, `((a*|b)*c)*`. 

The EBNF definition is as follows:

```
expr    ::= contact { "|" contact }
contact ::= factor { factor }
factor  ::= atom [ "*" ]
atom    ::= char | "(" expr ")"
```

Since the regular expression language provides a higher-level framework for generating arbitrary regular languages, defining its universal structure requires a context-free grammar (CFG). 

Consequently, because this syntax is context-free, a specifically designed pushdown automaton (PDA) is employed to parse arbitrary regular expressions into an abstract syntax tree (AST). This AST represents the underlying regular grammar and serves as the basis for constructing a concrete finite automaton.

### Parser & Abstract Syntax Tree (AST) 

According to the EBNF definition, the `Parser` parses the expression using the Recursive descent algorithm and returns an abstract syntax tree as follows:

- AST of `((a*|b)*c)*`

```
[Contact 1]: 
    [Repeat 15]: 
        [Group 14]: 
            [Contact 3]: 
                [Repeat 12]: 
                    [Group 11]: 
                        [Alter 8]: 
                            Left:
                            [Contact 5]: 
                                [Repeat 7]: 
                                    [Char 6]: a
                            Right:
                            [Contact 9]: 
                                [Char 10]: b

                [Char 13]: c
```

###  Builder & Nondeterministic finite automaton (NFA)

The builder takes the Abstract Syntax Tree (AST) and applies Thompson's construction algorithm to generate the NFA.

```mermaid
flowchart
    N_BEGIN["BEGIN"] --> N1
    N13["(13)<br/>IN<br/>Char 6"] --> N15
    N14["(14)<br/>OUT<br/>Char 6"] --> N13
    N14["(14)<br/>OUT<br/>Char 6"] --> N12
    N15["(15 Char: a)<br/>"] --> N14
    N11["(11)<br/>IN<br/>Repeat 7"] --> N13
    N11["(11)<br/>IN<br/>Repeat 7"] --> N12
    N12["(12)<br/>OUT<br/>Repeat 7"] --> N10
    N16["(16)<br/>IN<br/>Char 10"] --> N18
    N17["(17)<br/>OUT<br/>Char 10"] --> N10
    N18["(18 Char: b)<br/>"] --> N17
    N9["(9)<br/>IN<br/>Alter 8"] --> N11
    N9["(9)<br/>IN<br/>Alter 8"] --> N16
    N10["(10)<br/>OUT<br/>Alter 8"] --> N8
    N7["(7)<br/>IN<br/>Group 11"] --> N9
    N8["(8)<br/>OUT<br/>Group 11"] --> N7
    N8["(8)<br/>OUT<br/>Group 11"] --> N6
    N5["(5)<br/>IN<br/>Repeat 12"] --> N7
    N5["(5)<br/>IN<br/>Repeat 12"] --> N6
    N6["(6)<br/>OUT<br/>Repeat 12"] --> N19
    N19["(19)<br/>IN<br/>Char 13"] --> N21
    N20["(20)<br/>OUT<br/>Char 13"] --> N4
    N21["(21 Char: c)<br/>"] --> N20
    N3["(3)<br/>IN<br/>Group 14"] --> N5
    N4["(4)<br/>OUT<br/>Group 14"] --> N3
    N4["(4)<br/>OUT<br/>Group 14"] --> N2
    N1["(1)<br/>IN<br/>Repeat 15"] --> N3
    N1["(1)<br/>IN<br/>Repeat 15"] --> N2
    N2["(2)<br/>OUT<br/>Repeat 15"] --> N0
    N0["END"]
```

### Matching engine

Now that we have the NFA, there are multiple ways to matching the target string.

#### Backtrack engine

This engine walks through only one path at a time. 

However, since some states in the NFA have two outgoing transitions, it pushes the second state onto a global backtrack stack as a backup path. 

When a path fails, the engine pops a backup path and tries again.

A `ReDoS` attack can be reproduced using this engine. When matching `(a*)*` against `aaaaab`, the more `'a's` there are in the target string, the slower the engine is to find the result.

This happens because it tries every possible combination of `'a's`, such as `(a)(a)(a)(a)(a)`, `(aaaaa)`, and `(aa)(a)(aa)`. The following two engines do not have this issue.

#### Parallel engine

This engine does not backtrack. Instead, it finds all reachable next states at once and stores them in a set.

The algorithm should pass through ε-state, but stop at other types of states. 

If any of these states accept the character, it advances one character in the string, computes all reachable next states from the accepted states, and tries the next character. 

If all characters in the string are consumed and the END state is in the final active set, the engine returns true.

#### DFA engine (Deterministic finite automaton)

This engine converts the NFA to a DFA first, and then matches the target string using the DFA.

DFA-based string matching is much simpler and faster than NFA-based matching.

The DFA for `((a*|b)*c)*`, converted from its NFA.

```mermaid
flowchart LR
    start((*)) --> S0
    S0 --> stop((( )))

    S1 -- "c" --> S0
    S1 -- "b, a" --> S1
    S0 -- "b, a" --> S1
    S0 -- "c" --> S0
```
