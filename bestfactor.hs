prime_length = 1000


isprime p
  | p<2 = False
  | p==2 = True
  | mod p 2 == 0 = False
  | otherwise = ip p 3
  where ip q r
          | (r*r)>q = True
          | mod q r == 0 = False
          | otherwise = ip q (r+2)

order n p
  | mod n p == 0 = 0
  | otherwise = 1 + order (div n p) p

primes = take prime_length $ filter isprime [2..]

order_sequence r m p
