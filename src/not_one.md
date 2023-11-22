# Sum to any integer

If the sequence sums to an integer (not neccessarily 1)

\\[\begin{split}\begin{align} d \& = \mathcal{R} \lbrace v\_1, v\_2, \dots, v\_k \rbrace \\\\ \& = \frac{1}{v\_1}+\frac{1}{v\_2}+\dots+\frac{1}{v\_k} \end{align}\end{split} \\]
Then we can divide by *d*:
\\[\begin{split}\begin{align} 1 \& = \frac{1}{d \times v\_1}+\frac{1}{d \times v\_2}+\dots+\frac{1}{d \times v\_k}  \\\\ \& = \mathcal{R} \lbrace d \times v\_1, d \times v\_2, \dots, d\times v\_k \rbrace  \end{align}\end{split} \\]

### Injective

So every sequence 
\\[\mathcal{R\_k(\lbrace v\_i\rbrace)}=d \quad \text{for}\quad i =1 \rightarrow k\\] 
has a corresponding \\[\mathcal{R\_k(\lbrace d \times v\_i\rbrace)}=1 \quad \text{for}\quad i =1 \rightarrow k\\] 
of the same length.

### Common divisor

If a sequence that sums to 1 has terms with a common divisor *d*, i.e. \\[v\_i=d\times u\_i \quad \text{for all} \quad i=1 \rightarrow k \\]

\\[\begin{split}\begin{align} 1 = \& \mathcal{R\_k(\lbrace v\_0, v\_1, \dots , v\_k \rbrace )} \\\\ = \& \mathcal{R\_k(\lbrace d\times u\_0, d\times u\_1, \dots , d\times u\_k \rbrace )} \\\\ = \& \frac{1}{d \times u\_0}+\frac{1}{u\_1}+\dots+\frac{1}{d\times u\_k} \\\\ = \& \frac{\mathcal{R\_k(\lbrace u\_0, u\_1, \dots , u\_k \rbrace )}}{d}\end{align}\end{split}\\]

So a sequence with a common divisor summing to 1 has a corresponding:

\\[\mathcal{R\_k(\lbrace u\_0, u\_1, \dots , u\_k \rbrace )}=d\\]


### Relative prime

In: \\[\mathcal{R\_k(\lbrace v\_i\rbrace )}=1\\]
If
\\[ d \mid v\_i \quad \text{for all} \quad i=1 \rightarrow k\\] I.e. \\(v\_i\\) are not relatively prime, then

\\[\mathcal{R\_k(\lbrace\frac{v\_i}{d}\rbrace)}=d\quad \text{for}\quad i =1 \rightarrow k\\]

### Solution space

To find solutions for:
\\[\mathcal{R\_k(\lbrace v\_i\rbrace )}=d\quad \text{for}\quad i =1 \rightarrow k\\]
We need only consider:
\\[v\_i\quad \text{where} \quad v\_i \bmod{d}=0 \\]

