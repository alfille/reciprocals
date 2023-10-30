# Modulo Equivalence Classes

Although we've proven the base case: [Straight reciprocol sequence](straight.md) and extended it to [even](even.md) and [odd](odd.md) sequences, there is a more general set of sequences to explore.

## Modulo arithmetic

For a given __base__, the (infinite) set of numbers that give the same remainder on integer division are an *equivalence class*.

### Examples, base=3

#### Remainder 0
\\[ v \equiv 0 \bmod{3}\\]

\\[v=3,6,9,12,15,\dots\\]

#### Remainder 1
\\[ v \equiv 1 \bmod{3}\\]

\\[v=4,7,10,13,16,\dots\\]

#### Remainder 2
\\[ v \equiv 2 \bmod{3}\\]

\\[v=5,8,11,12,17,\dots\\]

#### Remainder 3

Same as __Remainder 0__ since 3 is a member of that equivalence class.

### General form

* base *m*
* remainder *r*
* denominator *v*

\\[v=r+i\times m\quad \text{where}\quad i=1,2,3,\dots\\]

## Modulo proof strategy

Recasting the [proof strategy](strategy.md):

#### Condition 0
New condition implicit in __straight__ sqquence:

*r* and *m* are *relatively prime*

Otherwise if \\((r,m)=d\\) then
\\[\frac{1}{r+m}+\frac{1}{r+2m}+\frac{1}{r+3m}+\dots=\frac{1}{d}\times\lbrace \frac{1}{r/d+m/d}+\frac{1}{r/d+2m/d}+\frac{1}{r/d+3m/d}+\dots \rbrace\\]

which is a more restrictive version of another modulo sequence with relatively prime components
\\[v \equiv (r/d) \bmod (m/d)\\]

### Condition 0*

Note the special case \\(r=0\\) which means greatest common divisor  \\((r,m)=(0,m)=m\\)

\\[\frac{1}{m}+\frac{1}{2m}+\frac{1}{3m}+\dots=\frac{1}{m}\times\lbrace \frac{1}{1}+\frac{1}{2}+\frac{1}{3}+\dots \rbrace\\]

### Condition I

\\[k\times p= r + i \times m \quad \text{where} \quad 0 \lt i \le p\quad\text{for some integer}\\; k\\]

This implies *p* and *m* are *relatively prime* 
\\( (p,m)=1 \\)

### Condition II

We will need to check the sums

\\[S(1),S(2),\dots,S(i-1)\\]

Since \\(i\le p\\) this is a bounded task

### Condition III

\\[\text{If}\quad p\^e\mid v\_m \quad \text{for some smallest}\\; m\\quad\text{and}\quad p\^e\mid v\_n \quad\text{for the smallest}\quad n\gt m\quad \text{then}\quad p\^{e+1}\mid v\_n \\]

Let

\\[v\_m=k\times p\^e=r+i\times m\\]

The next term divisible by \\(p\^e\\) is \\(v\_n+m\times p\^e \\)  ( since *p* and *m* are *relatively prime* )

So

\\[v\_n=v_m+m\times p\^e = k\times p\^e+ m\times p\^e=(k+m)\times p\^e\\]

The requirement that \\( p\^{e+1}\mid v\_n\\) means

\\[ k_2\times p\^{e+1}=(k+m)\times p\^e\\]

or

\\[k_2\times p = k+m \quad \text{for some}\quad k\_2\gt 0\\]