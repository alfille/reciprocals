# Rational numbers and sums

## Rational number

A *rational number* is a fraction of the form \\( a /b \\) where *a* and *b* are positive integers.

### Reduced form

All common factors of *a* and *b* can be eliminated so

\\[ \text{Reduced form} \quad \frac{a}{b} \quad \text{where} (a,b)=1 \\] 
I.e. *a* and *b* are *relatively prime*

### Rational sum

\\[\frac{a}{b}+\frac{c}{d}=\frac{a \times d+c \times b}{b \times d} \tag{B1}\\]

## Integer rational

### Condition for integer

* A __reduced fraction__ \\( a /b\\) is an integer only if \\(b=1\\) . (We are ignoring zero as a numerator)
* An arbitrary fraction \\( a /b\\) is an integer only if
\\[ b \mid a \\, \text{,  equivalently:} \quad (a,b)=b \\]

### Integer sum

The sum:

\\[\frac{a}{b}+\frac{c}{d}=\frac{a \times d+c \times b}{b \times d}\\]


can be an integer only if
\\[{b\times d} \mid (a \times d + c \times b)\\]

----

We can be more specific if we know that \\(\frac{a}{b}\\) and \\( \frac{c}{d} \\) are *reduced fractions* I.e.
\\[(a,b)=1 \quad \text{and} \quad (c,d)=1\\]

From the [previous section (A3)](basic_theory.md) that implies
\\[{b} \mid (a \times d + c \times b)\\]
\\[{d} \mid (a \times d + c \times b)\\]
which [implies (A2)](basic_theory.md)
\\[{b} \mid (a \times d)\\]
\\[{d} \mid (c \times b)\\]
Now since \\((a,b)=1 \quad \text{and} \quad (c,d)=1\\) from [(A1)](basic_theory.md)
\\[{b} \mid d\\]
\\[{d} \mid b\\]
Only true if \\(b=d\\)

So the requirement for the sum of reduced fractions being an integer is that they have the __same denominator__.

