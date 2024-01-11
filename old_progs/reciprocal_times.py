#!/usr/bin/env python3

import subprocess
import os
import signal
import sys
import argparse
import sqlite3

def test( level, vals ):
    print([SolutionSpace.gProgram,"-s",str(SolutionSpace.gLength),"--timer"]+[str(v) for v in vals])
    with subprocess.Popen(
        [SolutionSpace.gProgram,"-s",str(SolutionSpace.gLength),"--timer"]+[str(v) for v in vals],
        stdout=subprocess.PIPE,
        preexec_fn=os.setsid
        ) as process:
        try:
            output=process.communicate(timeout=SolutionSpace.gTimeout)[0]
            try:
                timer,counter,status = output.decode('utf-8').split(',')
                print(f"{timer} sec, {counter} solutions, status: {status}")
            except:
                print(f"Return: {output.decode('utf-8')}")
        except subprocess.TimeoutExpired:
            print("Timeout")
            os.killpg(process.pid, signal.SIGINT) # send signal to the process group
            output=process.communicate()[0]

class SolutionSpace:
    gTimeout = 120
    gLength = 3
    gProgram = "./reciprocals6"
    def __init__(self):
        self.filename = f"Reciprocal_{SolutionSpace.gLength}.db"
        self.level = 0
        self.solution = {}
        self.con = sqlite3.connect(self.filename)
        cursor = self.con.cursor()
        cursor.execute(f"CREATE TABLE IF NOT EXISTS solve(values TEXT PRIMARY KEY, level INTEGER NOT NULL, time REAL NOT NULL, count INTEGER NOT NULL, status TEXT NOT NULL)")

    def single_try( self, level, vals ):
        cursor = self.con.cursor()
        try:
            timer,counter,status = subprocess.run(
                [SolutionSpace.gProgram,"-s",str(SolutionSpace.gLength),"--timer"]+[str(v) for v in vals],
                timeout = SolutionSpace.gTimeout,
                capture_output = True
                ).stdout.decode('utf-8').split(",")
            cursor.execute(f"INSERT INTO solve VALUES ({','.join(vals)}, {level}, {timer}, {counter}, 'done'")
        except subprocess.TimeoutExpired:
            cursor.execute(f"INSERT INTO solve VALUES ({level}, -1, 0, 'timeout', {',',join(vals)}")
        self.con.commit()
            

def MakeCommand( vals=[] ):
    if vals == None:
        vals = []
    return [SolutionSpace.gProgram,"-s",SolutionSpace.gLength,"--timer"]+vals

# Custom argparse type representing a bounded int
# from https://stackoverflow.com/questions/14117415/how-can-i-constrain-a-value-parsed-with-argparse-for-example-restrict-an-integer
class IntRange:

    def __init__(self, imin=None, imax=None):
        self.imin = imin
        self.imax = imax

    def __call__(self, arg):
        try:
            value = int(arg)
        except ValueError:
            raise self.exception()
        if (self.imin is not None and value < self.imin) or (self.imax is not None and value > self.imax):
            raise self.exception()
        return value

    def exception(self):
        if self.imin is not None and self.imax is not None:
            return argparse.ArgumentTypeError(f"Must be an integer in the range [{self.imin}, {self.imax}]")
        elif self.imin is not None:
            return argparse.ArgumentTypeError(f"Must be an integer >= {self.imin}")
        elif self.imax is not None:
            return argparse.ArgumentTypeError(f"Must be an integer <= {self.imax}")
        else:
            return argparse.ArgumentTypeError("Must be an integer")

def main( sysargs ):
    # Command line first
    parser = argparse.ArgumentParser(
        prog="Reciprocals timing",
        description="Find which sequences can be solved in reasonable time",
        epilog="Contact Paul Alfille for questions about this program")
    parser.add_argument('-l','--length',
        metavar="Length",
        required=False,
        default=SolutionSpace.gLength,
        const=SolutionSpace.gLength,
        dest="length",
        type=IntRange(3),
        nargs='?',
        help='Number of terms in solutions'
        )
    parser.add_argument('-t','--timeout',
        metavar="Timeout",
        required=False,
        default=SolutionSpace.gTimeout,
        const=SolutionSpace.gTimeout,
        dest="timeout",
        type=IntRange(10,6000),
        nargs='?',
        help='Solution timeout (in seconds)'
        )
    parser.add_argument('-p','--program',
        metavar="Program",
        required=False,
        default=SolutionSpace.gProgram,
        dest="program",
        type=str,
        nargs='?',
        help='Program to invoke for solving'
        )
    parser.add_argument("vals",
        metavar="VALUES",
        type=IntRange(2),
        #action='append',
        nargs="*",
        help="Preset values"
        )
        
    args=parser.parse_args()
    print(sysargs,args)
    SolutionSpace.gLength = args.length
    SolutionSpace.gTimeout = args.timeout
    SolutionSpace.gProgram = args.program

    test( 0, args.vals )
    
if __name__ == "__main__":
    sys.exit(main(sys.argv))
else:
    print("Standalone program")
    
