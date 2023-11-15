# Solution Strategy

The sequence

\\[\lbrace v\_1, v\_2,\dots,v\_n\rbrace\\]

We will have partial sums of the form:

\\[ S(n)=\frac{1}{v\_1}+\frac{1}{v\_2}+\frac{1}{v\_3}+\\dots+\frac{1}{v\_n} \\]

Choose a *p* so that 

\\[ S(n) = \frac{a}{b\times p\^e} \quad \text{where}\quad (a,p)=1\quad\text{and}\quad(b,p)=1\\]

## Math Induction

We will be summing the series \\( 1/v\_1, 1/v\_2,\dots\\) with a test prime *p*

\\[\text{Let}\quad S(n)=1/v\_1+1/v\_2+\dots+1/v\_n\\]

### Condition I - Existence

In the series:

\\[v\_1,v\_2,\dots\\]

For some prime *p* there is a minimal element \\(v\_m\\) where
\\[p\mid v\_m\\]

### Condition II - Start

None of the terms \\(S(1),\dots,S(v\_{m-1})\\) are integers

Note that \\(S(v\_m)=\frac{a}{p\times b}\\)is non-integer since \\(1/v\_m\\) was the first term with *p* in the denominator.

### Condition III Induction

Suppose that \\(v\_m\\) is the first term where

\\[p\^e\mid v\_m\quad\text{thus}\\;S(v\_m)=\frac{a}{b\times p\^e}\quad\text{where}\\;p\nmid a,\\; p\nmid b\\]

Let \\(v\_n\\) be the __next__ term where \\(p\^e\mid v\_n\\)

Our condition is that \\[p\^{e+1}\mid v\_n\\] 

Thus there is an ever-increasing power of *p* in the denominator of \\(S(1),S(2),\dots\\)
