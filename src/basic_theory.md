# Basic Number Theory

## Integer Domain

We will be using positive integers, and by extension rational numbers, exclusively. Number theory has been extended to other domains, but we won't need that.

A brief synopsis of the basic parts of Number Theory that we will use follows below.

## Divisibility

We say that "a divides b" 
\\[ a \mid b \\] 
when integer a divides b cleanly. In other words \\( a \div b \\) is an integer.

More formally, there is an integer "c" so that 

\\[ a \times c = b\\]

* *1* divides everything
* \\( a \mid a \\) for all positive integers

### Indivisibility

If a does not divide b, we write

\\[ a \nmid b \\]

For instance:

\\[ \text{If} \quad a \mid (b \times c) \quad \text{and} \quad a \nmid b \quad \text{then} \quad a \mid c \\]

### Sum

\\[ \text{If} \quad a \mid (b+c) \quad \text{and} \quad a \mid b \quad \text{then} \quad a \mid c \\]

### Product

\\[ \text{If} \quad (a \times b) \mid c \quad \text{then} \quad a \mid c \quad \text{and} \quad b \mid c \\]

### Equality

\\[ \text{If} \quad a \mid b \quad \text{and} \quad b \mid a \quad \text{then} \quad a = b \\] 

### Euclid's division lemma

Given any 2 integers *a*, *b* there are integers *q* and *r* such that

\\[a = q \times b + r  \qquad  \text{where} \quad  0 \le r \lt b \\]

* *q* is the quotient
* *r* is the remainder
* if \\(r=0\\) then \\( b \mid a \\)
* if \\(r \ne 0 \\) then \\( b \nmid a \\)

### Modulus
Another way of writing this is using *modulus* ( *mod* operator)
\\[ a = r \mod b \\]

## Primes

Numbers that have no divisors (except themselves and *1* are __prime__)

### Fundamental Theorem of Arithmetic
Every integer has a unique factorization into *primes*. Pretty intuitive, although a formal proof takes some work.

### Prime order

Number of times prime *p* divides a number. I.e.

\\[ \\text{Let}\quad o={ord}_{p}(n) \\]
\\[ p\^{o} \mid n \quad \text{while} \quad p\^{o+1}\nmid n \\]

### Prime divisor

For prime *p*:

\\[ \text{If} \quad p \mid (a \times b ) \quad \text{then} \quad p \mid a \quad \text{and/or} \quad p \mid b \\]

## Greatest Common Divisor

Every pair of integers has a *greatest common divisor*. Formally

\\[ \text{greatest common divisor: } d \mid a \text{ and } d \mid b \\]
For any other mutual divisor:
\\[ \text{if} \quad c \mid a \quad \text{and} \quad c \mid b \quad \text{then} \quad c \mid d \\]
We typically write:
\\[ d = (a,b) \\]

### Relatively prime

If *a* and *b* have no factors (primes) in common then they are __relatively prime__. I.e.
\\[(a,b)=1\\]





