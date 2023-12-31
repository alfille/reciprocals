# Odd

Sequence

\\[\lbrace3,5,7,\dots,2\times n+1\rbrace\\]

\\[S(n)=\frac{1}{3}+\frac{1}{5}+\frac{1}{7}+\dots+\frac{1}{2 \times n+1}\tag{P1}\\]

* Prime 
	* \\(p=3\\)
* Condition I
	- \\( \lbrace \frac{1}{p\^1}=\frac{1}{3}\rbrace  \mid \lbrace S(1)=\frac{1}{3} \rbrace\\)
* Condition II
	- \\(\frac{1}{3}\\) is not an integer
* Condition III 
	* for any term \\(3\^e\\) added, the next term with \\(3\^e\\) is \\(2 \times 3\^e\\) 
	* but that is even 
	* so the next term after that is \\(3 \times 3\^e=3\^{e+1}\\) 
* Table of Terms
	
Term |sequence| \\(ord\_3(term)\\)|\\(ord\_3(S)\\)
---|---|---|---|
\\(\frac{1}{3}\\)|3|1|1
\\(\frac{1}{5}\\)|5|0|1
\\(\frac{1}{7}\\)|7|0|1
\\(\frac{1}{9}\\)|9|2|2
\\(\frac{1}{11}\\)|11|0|2
\\(\frac{1}{13}\\)|13|0|2
\\(\frac{1}{15}\\)|15|1|2
\\(\frac{1}{17}\\)|17|0|2
\\(\frac{1}{19}\\)|19|0|2
\\(\frac{1}{21}\\)|21|1|2
\\(\frac{1}{23}\\)|23|0|2
\\(\frac{1}{25}\\)|25|0|2
\\(\frac{1}{27}\\)|27|3|3
\\(\frac{1}{29}\\)|29|0|3
