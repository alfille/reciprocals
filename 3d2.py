# based on https://pythonprogramming.net/3d-graphing-pandas-matplotlib/

import pandas as pd
from pandas import DataFrame
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.patches as patches
import numpy as np
import argparse
import sys


class Data:
    def __init__(self,filename):
        df = pd.read_csv(filename, names=['Diff','Terms_left','Denominator','Count','Last'],header=None)
        self.df = {}
        self.tl = list(set(df['Terms_left']))
        for tl in self.tl:
            self.df[tl] = df[df['Terms_left']==tl]
            print(self.df[tl].head())
    
    def colorset( self, r_df ):
        # return color,handles
        levels, categories = pd.factorize(r_df)
        colors = [plt.cm.tab10(i) for i in levels] # using the "tab10" colormap
        handles = [patches.Patch(color=plt.cm.tab10(i), label=c) for i, c in enumerate(categories)]
        return ( colors, handles )
        
    def threeD( self) :
        for tl in self.tl:
            self.threeD_1(tl)

    def twoD( self ):
        for tl in self.tl:
            self.diff_den(tl)
            self.diff_den2(tl)

    def threeD_1( self,tl ):
        X = 'Diff'
        Y = 'Denominator'
        Z = 'Count'
        R = 'Terms_left'
        S = 'Last'

        x = self.df[tl][X]
        y = self.df[tl][Y]
        z = self.df[tl][Z]
        r = self.df[tl][R]
        s = self.df[tl][S]

        colors, handles = self.colorset( s )

        threedee = plt.figure().add_subplot(projection='3d')
        threedee.scatter( x.apply(np.log), y, z.apply(np.log), c=colors )
        threedee.set_xlabel(X)
        threedee.set_ylabel(Y)
        threedee.set_zlabel(Z)
        plt.legend(handles=handles, title=S)
        plt.title( f"Terms Left {tl}")
        plt.show()

    def diff_den( self,tl ):
        X = 'Diff'
        Y = 'Denominator'
        R = 'Terms_left'
        S = 'Last'

        x = self.df[tl][X]
        y = self.df[tl][Y]
        r = self.df[tl][R]
        s = self.df[tl][S]

        colors, handles = self.colorset( s )

        #threedee = plt.figure()
        ax=plt.scatter( x.apply(np.log), y, c=colors )
        #ax.set_xlabel(X)
        #ax.set_ylabel(Y)
        plt.legend(handles=handles, title=S)
        plt.title( f"Terms Left {tl}")
        plt.show()

    def diff_den2( self,tl ):
        X = 'Diff'
        Y = 'Denominator'
        R = 'Terms_left'
        S = 'Last'

        x = self.df[tl][X]
        y = self.df[tl][Y]
        r = self.df[tl][R]
        s = self.df[tl][S]

        colors, handles = self.colorset( s )

        #threedee = plt.figure()
        ax=plt.scatter( x.apply(np.log), y.apply(np.log), c=colors )
        #ax.set_xlabel(X)
        #ax.set_ylabel(Y)
        plt.legend(handles=handles, title=S)
        plt.title( f"Terms Left {tl}")
        plt.show()


def main( sysargs ):
    # Command line first
    parser = argparse.ArgumentParser(
        prog="Egyptian fractions graphing",
        description="Plot output from reciprocals_fractions (CSV files)",
        epilog="Contact Paul Alfille for questions about this program\nhttps://github.com/alfille/reciprocals")
    parser.add_argument('-3','--3d',
        required=False,
        action="store_true",
        dest="d3",
        help='Show 3D graph'
        )
    parser.add_argument('-2','--2d',
        required=False,
        action="store_true",
        dest="d2",
        help='Show 2D graph'
        )
    parser.add_argument('csv',
        nargs='?',
        metavar='CSV_FILE',
        default='x8.csv',
        help='CSV file to use'
        )
        
    args=parser.parse_args()
    #print(sysargs,args)

    dataset = Data(args.csv)

    if not args.d3 and not args.d2:
        args.d3 = True
        args.d2 = True

    if args.d3:
        dataset.threeD()

    if args.d2:
        dataset.twoD()

if __name__ == "__main__":
    sys.exit(main(sys.argv))
else:
    print("Standalone program")
    

