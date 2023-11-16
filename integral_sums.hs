-- find reciprocals sums that are integers. 
-- the cannnonical example is 1/2 + 1/3 + 1/6 =1
-- Paul H Alfille 2023

data SumTest a = Low | Seq a | High deriving (Show)

data TestSeq = TestSeq [ index   :: Int
               , leng    :: Int
               , val     :: Integer
              -- , sum     :: Rational
               , pat :: Integer 
               , x :: Int ] 
               

