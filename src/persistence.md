# Prime Power Persistence

## Summary

We will show that if a fractions has a prime to a power in the denominator, the sum with another fraction will also have that prime to a power unless:

* the second term has the prime to a higher power (that will win out)
* the second term has the prime to the same power -- unclear result

## Rational sum

From the [previous section](rational_sum.md)

\\[\frac{a}{b}+\frac{c}{d}=\frac{a \times d + b \times d}{b \times d}\\]

## Prime factor



Choose a prime number *p* such that

\\[p \nmid a,\quad p \nmid b, \quad p\nmid c \quad p \nmid d\\]

i.e. *p* has no factors in common with *a,b,c,d*. The fractions need not be in reduced form.

What is

\\[\frac{a}{b\times p\^e}+\frac{c}{d\times p\^f}\\]

Arbitrarily assume \\(e \ge f\\)

\\[\begin{split}\begin{align}\frac{a}{b\times p\^e}+\frac{c}{d\times p\^f} \& =\frac{a \times d \times p\^f+c \times b \times p\^e}{b \times d \times p\^{e+f}} \\\\ \& = \frac{a \times d+c \times b \times p\^{e-f}}{b \times d \times p\^{e}}\end{align}\end{split}\tag{C1}\\]

Note that *p* appears only in the second term of the numerator, and only if \\(e-f \ne 0\\)

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

## Utility

By finding persistent prime factors, we can prove a sequence doesn't sum to an integer.