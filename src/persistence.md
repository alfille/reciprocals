# Prime Power Persistence

## Rational sum

From the [previous section](rational_sum.md) we know that (for reduced fractions)

\\[\frac{a}{b}+\frac{c}{d}=\frac{a \times d+c \times b}{b \times d}\\]
 is an integer only if \\(b=d\\)
 
## Prime factor

Choose a number *p* such that

\\[(p,a)=1\quad(p,b)=1\quad(p,c)=1\quad(p,d)=1\\]

i.e. *p* has no factors in common with *a,b,c,d*. The fractions need not be in reduced form.

What is

\\[\frac{a}{b\times p\^e}+\frac{c}{d\times p\^f}\\]

Arbitrarily assume \\(e \ge f\\)

\\[\frac{a}{b\times p\^e}+\frac{c}{d\times p\^f}=\frac{a \times d \times p\^f+c \times b \times p\^e}{b \times d \times p\^{e+f}}\\]

or
\\[\frac{a}{b\times p\^e}+\frac{c}{d\times p\^f}=\frac{a \times d+c \times b \times p\^{e-f}}{b \times d \times p\^{e}}\\]

## Two cases

### Inequality

If \\(e \gt f\\) then
\\[p \nmid (a \times d+c \times b \times p\^{e-f})\\]
since
\\[p \nmid (a \times d) \quad \text{and} \quad p \mid (c \times b \times p\^{e-f})\\]
So we have a sum of the form
\\[\frac{x}{y\times p\^e}\quad\text{where}\quad p\nmid x\quad\text{and}\quad p\nmid y\\]

In other words, the term \\(p\^e\\) __persists__ in the denominator and the fraction cannot be an integer

### Equality

If \\(e = f\\) then  
\\[a \times d+c \times b \times p\^{e-f}=a \times d+c \times b \\]

And it is possible that 

\\[p\^e \mid (a \times d+c \times b) \\]

unless there are other restrictions. 

Thus the \\(p\^e\\) term does __not persist__, and we cannot state that the sum is not an integer.