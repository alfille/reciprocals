# Consecutive Sequences

Consider (in [sequence notation](seq.md)):

\\[\begin{split} \mathcal{T\_k(n)} =\mathcal{R} \lbrace n, n+1, n+2, \dots, n+k-1 \rbrace \\\\ = \frac{1}{n}+\frac{1}{n+1}+\frac{1}{n+2}+\dots+\frac{1}{n+k-1}  \\\\ \text{where} \quad k \gt 1\end{split} \\]

A consecutive sequence of *k* length starting at *n.*

## Maximum sequence

If \\(\mathcal{R\_k(n)}\\) is any squence of length *k* starting at *n* then
\\[ \mathcal{T\_k(n)} \ge \mathcal{R\_k(n)} \\]

since every element of \\(\mathcal{T\_k(n)}\\) is less or equal to the same element of \\(\mathcal{R\_k(n)}\\)

---

Also,

\\[\mathcal{T\_k(n)} \gt \mathcal{T\_{k}(n+1)} \ge \mathcal{R\_{k}(n+1)}\\]

## Limits

### length

\\[ \lim\_{k \to \infty }\mathcal{T\_k(n)}=\infty \quad \text{for all} \\, n\\]

\\[ \lim\_{k \to \infty }\mathcal{R\_k(n)} \ne\infty \quad \text{for some sequences}\\]
e.g. \\[\mathcal{R\_k(\lbrace 2\^{-i}\rbrace)}\lt 1\quad \text{for all}\\,k\\]

### Starting value

\\[ \lim\_{n \to \infty }\mathcal{T\_k(n)}=0 \quad \text{for all} \\, k\\]
\\[ \lim\_{n \to \infty }\mathcal{R\_k(\lbrace v\_i\rbrace})=0 \quad \text{for all} \\, k\quad \text{where}\quad v\_1=n\\]

## Estimate of value

Since
\\[\frac{1}{n}+\frac{1}{n}+\dots \gt \frac{1}{n}+\frac{1}{n+1}+\dots\\]

We have a upper bound estimate:

\\[ \frac{k}{n} \ge \mathcal{T\_k(n)} \quad \text{with equality only for} \\quad k=1\\] 

----
More accurate estimate:

![](images/Tk_estimate.png)

Upper and lower bounds for estimate:

\\[\frac{k}{n} \gt \mathcal{T\_k(n)} \approx \frac{k}{n+k/2} \gt \frac{k}{n+k-1}\\]