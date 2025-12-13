import re
import sys

if len ( sys.argv )<3:
  print ( 'Usage: generateExcerptGlassIncFile.py <inputfilename> <outputfilename> [list of defines to look for]' )
  print ( '       If list is empty, all values are included' )
  exit()

f1 = open( sys.argv[ 2 ], 'w' )

defines = sys.argv[ 3:len( sys.argv ) ]

def addDefine( f1, line ):
  parts = re.split( ' .equ ', line )
  
  if parts[ 1 ].startswith( '0b' ) or parts[ 1 ].startswith( '0B' ):
    parts[ 1 ] = "%" + parts[ 1 ][ 2: ]
  
  f1.write( parts[ 0 ] + ': equ ' + parts[ 1 ] )

with open( sys.argv[ 1 ], 'r' ) as f2:
  for line in f2:
    if defines is None or len( defines )==0:
        if not line.strip().startswith( ".define" ):
          addDefine( f1, line )
    else:
      for define in defines:
        if line.strip().startswith( define+' ' ):
          addDefine( f1, line )
            
f2.close()
f1.close()

exit()