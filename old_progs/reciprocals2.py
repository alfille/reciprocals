# Reciprocals
# see https://github.com/alfille/reciprocals
# Paul H Alfille 2023
# MIT license

# version 2 -- use reduced fractions

from math import gcd

class byLength:
    # All the sequences summing to 1 for of given length
    def __init__(self, length ):
        self.max_index = length-1
        self.val = [0]*length
        self.num = [0]*length
        self.den = [0]*length
        self.first_stage(2)
                
    def last_stage( self, in_val ):
        num = self.num[self.max_index-1]
        den = self.den[self.max_index-1]
        diff = den - num
        if (diff == 1) and ( den >= in_val ) :
            self.val[self.max_index] = den
            self.emit()
            return den != in_val
        return den >= diff * in_val

    def first_stage( self, in_val ):
        val = in_val
        while True :
            #print("first loop",0,val)
            self.val[0] = val
            self.num[0] = 1
            self.den[0] = val
            if self.stage( 1, val+1 ):
                val += 1
            else:
                return False

    def stage( self, index, in_val ):
        if index == self.max_index:
            return self.last_stage( in_val )
        val = in_val
        while True :
            #print("middle loop",index,val)
            self.val[index] = val
            num = val * self.num[index-1] + self.den[index-1]
            den = val * self.den[index-1]
            g = gcd( num, den )
            self.num[index] = num  // g
            self.den[index] = den // g
            if self.num[index] >= self.den[index] or self.stage( index+1, val+1 ):
                val += 1
            else:
                return val != in_val
                
    def emit( self ):
        print(self.val)
        
class CountbyLength(byLength):
    def __init__( self,length ):
        self.count = 0
        super().__init__( length )
        
    def emit(self):
        self.count += 1
        
    def show(self):
        print(f"Length= {self.max_index+1}, count={self.count}") 

for L in range( 3,10 ):
    CountbyLength(L).show()
               
