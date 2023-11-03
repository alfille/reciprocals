# General Modulo Solution

We will use the idea from the flawed [remainder 1 strategy](remainder1.md) but Acknowledge the possibility of composite numbers and the need to change primes at every step.

### Step 0

\\[v\_0=r+m\quad \text{where} \quad (m,r)=1 \rightarrow (m,v\_0)=1\\]
By the [Fundamental Theorem](basic_theory.md)
\\[v\_0={p\_0}\^{e\_0}\times {p\_1}\^{e\_1}\times \dots \quad \text{p's prime}\\]
\\[ \text{Let} \quad P\_0={p\_0}\^{e\_0} \quad \text{The choice of } p\_0 \text{ is arbitrary}\\]

Obviously this (first) term is non-integer. And \\(P\_0\\) is in the denominator.

### Step 1

Find the next term where \\( P_0 \mid v\_0+m \times P\_0\\) but stop just before. Thus \\(P\_0\\) has persistened in the denominator.

\\[\begin{split}\begin{align}v\_1 \& =v\_0 + m \times (P\_0-1) \\\\ \& = r+ m + m\times (P\_0-1)\end{align}\end{split}\\]
Obviously \\((m,v_1)=1\\) and \\(P\_0\\) persisted in all the terms so the sums to this point are non-integer.

Find the prime decomposition for \\(v\_1\\) and choose one of those primes to it's order \\(=P\_1\\). 

So for this last term, \\(P\_1\\) is in the denominator.

### Step 2

Find the next term where \\( P_1 \mid v\_1+m \times P\_1\\) but stop just before. Thus \\(P\_1\\) has persistened in the denominator.

\\[\begin{split}\begin{align}v\_2 \& =v\_1 + m \times (P\_1-1) \\\\ \& = v\_0 + m \times (P\_0-1) + m \times (P\_1-1) \\\\ \& = r+ m + m\times (P\_0+P\_1-2)\end{align}\end{split}\\]
Obviously \\((m,v_2)=1\\) and \\(P\_1\\) persisted in all these new terms so the sums to this point are non-integer.

We can again decompose \\(v_2\\) into primnes and choose a \\(P\_2\\)

Also note that
\\[v\_1\gt v\_0\\]

### Step n

Find the next term where \\( P_{n-1} \mid v\_{n-1}+m \times P\_{n-1}\\) but stop just before. Thus \\(P\_{n-1}\\) has persistened in the denominator.

\\[\begin{split}\begin{align}v\_n \& =v\_{n-1} + m \times (P\_{n-1}-1) \\\\ \& = v\_0 + m \times (P\_0-1) + m \times (P\_1-1) +\dots + m \times (P\_{n-1}-1)\\\\ \& = r+ m + m\times (P\_0+P\_1+\dots+P\_{n-1}-n)\end{align}\end{split}\\]
Obviously \\((m,v_n)=1\\) and \\(P\_{n-1}\\) persisted in all these new terms so the sums to this point are non-integer.

\\[v\_n\gt v\_{n-1} \gt \dots \gt v\_0 \\]

So \\(v\_n\\) is unbounded and non-integer partial sums is proven for all reciprocal modulo equivalence classes. 


