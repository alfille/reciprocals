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

order :: Int -> Int -> Int
order n p
  | mod n p == 0 = 0
  | otherwise = 1 + order (div n p) p

primes = take prime_length $ filter isprime [2..]

rising [] = []
rising [x] = [x]
rising (x:xs)
  | x<(head xs) = (x:rising xs)
  | otherwise = [x]

data Recip = Recip {
  modulus:: Int ,
  remainder:: Int,
  prime:: Int
  } deriving (Show)

data RStage = RStage {
  rEcip:: Recip,
  iNdex:: Int,
  vAlue:: Int
  } deriving(Show)

rOrder (RStage { rEcip= Recip {prime=p}, vAlue=n }) = order n p

nextStage (RStage (Recip m r p) i v) =
  RStage (Recip m r p) (i+1) (v + m * p^(i+1) ) 

firstStage rc@(Recip m r p) = (RStage rc 1 (tst r))
  where tst v
          | mod v p == 0 = v
          | otherwise = tst (v+m)
 
