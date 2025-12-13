import os
import argparse

parser = argparse.ArgumentParser( description='Specificed symbols are extracted from map-file, prefixed with filename + "_" and put into a .h-file')
parser.add_argument( 'input', help=".map input file, including path" )
parser.add_argument( 'output', help=".h input file, including path" )
parser.add_argument( 'symbolname', nargs='+', help="Name(s) of symbol(s) to be extracted",  )


args = vars( parser.parse_args() )


# =====================================
def cutExtension( s ):
    pos = s.rfind( '.' )
    if pos>=0:
        return s[ 0:s.rfind( '.' ) ] # cut the ext
    else:
        return s
    

# =====================================
def cutPath( s ):
    pos = s.rfind( os.sep )
    if pos>=0:
        return s[ s.rfind( os.sep )+1: ] # cut the ext
    else:
        return s


input = args[ 'input' ]
output = args[ 'output' ]
prefix = cutPath( cutExtension( input ) ) + '_'
allsymbols = args[ 'symbolname' ]

def is_hex(s):
	try:
		int(s, 16)
		return True
	except ValueError:
		return False

f1 = open( output, 'w' )

with open( input , 'r' ) as f2:
    for line in f2:
        line1 = line.strip()
        words = line1.split()

        if len( words ) > 1:
            if words[1] in allsymbols:
                if is_hex(words[0]):
                    f1.write( "#define " + prefix + words[ 1 ] + ' 0x' + words[ 0 ] )

f2.close()
f1.close()

exit()
