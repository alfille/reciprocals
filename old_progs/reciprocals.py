# Reciprocals
# see https://github.com/alfille/reciprocals
# Paul H Alfille 2023
# MIT license

class byLength:
    # All the sequences summing to 1 for of given length
    def __init__(self, length ):
        self.max_index = length-1
        self.val = [0]*length
        self.num = [0]*length
        self.den = [0]*length
        self.first_stage(2)
                
    def last_stage( self, in_val ):
        val = in_val
        while True :
            #print("last loop",self.max_index,val)
            self.val[self.max_index] = val
            self.num[self.max_index] = val * self.num[self.max_index-1] + self.den[self.max_index-1]
            self.den[self.max_index] = val * self.den[self.max_index-1]
            if self.num[self.max_index] < self.den[self.max_index]:
                # too small forever
                return val != in_val
            elif self.num[self.max_index] == self.den[self.max_index]:
                # success
                self.emit()
                return val != in_val
            val += 1

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
            self.num[index] = val * self.num[index-1] + self.den[index-1]
            self.den[index] = val * self.den[index-1]
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
               
