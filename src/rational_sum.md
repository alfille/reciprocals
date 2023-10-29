# Rational numbers and sums

## Rational number

A *rational number* is a fraction of the form \\( \frac{a}{b} \\) where *a* and *b* are positive integers.

### Reduced form

All common factors of *a* and *b* can be eliminated so

\\[ \text{Reduced form} \quad \frac{a}{b} \quad \text{where} (a,b)=1 \\] 
I.e. *a* and *b* are *relatively prime*

### Rational sum

\\[\frac{a}{b}+\frac{c}{d}=\frac{a \times d+c \times b}{b \times d}\\]

## Integer rational

### Condition for integer

* A __reduced fraction__ \\( \frac{a}{b}\\) is an integer only if \\(b=1\\) . (We are ignoring zero as a numerator or integer).
* An arbitrary fraction \\( \frac{a}{b}\\) is an integer only if
\\[ b \mid a \quad \text{, equivalently:} \quad (a,b)=b \\]

### Integer sum

The sum:

\\[\frac{a}{b}+\frac{c}{d}=\frac{a \times d+c \times b}{b \times d}\\]

where \\(\frac{a}{b}\\) and \\( \frac{c}{d} \\) are *reduced fractions* I.e.
\\[(a,b)=1 \quad \text{and} \quad (c,d)=1\\]

can be an integer only if
\\[{b\times d} \mid (a \times d + c \times b)\\]

From the [previous section](basic_theory.md) we know that that implies
\\[{b} \mid (a \times d + c \times b)\\]
\\[{d} \mid (a \times d + c \times b)\\]
which implies
\\[{b} \mid (a \times d)\\]
\\[{d} \mid (c \times b)\\]
Now since \\((a,b)=1 \quad \text{and} \quad (c,d)=1\\)
\\[{b} \mid d\\]
\\[{d} \mid b\\]
Only true if \\(b=d\\)

So the requirement for the sum of reduced fractions being an integer is that they have the __same denominator__.

