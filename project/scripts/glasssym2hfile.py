import re
import sys

if len ( sys.argv )<3:
  print ( 'Usage: glasssym2hfile.py <filename_symfile> <outputfilename> [excludefilename]' )
  print ( '       symbols existing in excludefilename(.inc) are stripped out from outputfile (.h)' )
  print ( '       symbols starting with _ are ignored' )
  print ( '       symbols including . are ignored' )
  exit()

# First build exclude list if there is any
exclude_list = []
if len( sys.argv )==4:
  with open( sys.argv[ 3 ], 'r' ) as f3:
    for line in f3:
      parts = re.split( ': equ ', line, 1 )
      if len( parts )<2:
        continue
      exclude_list.append( parts[ 0 ] )

# Then build the file, while checking against the exclude list
f1 = open( sys.argv[ 2 ], 'w' )
with open( sys.argv[ 1 ], 'r' ) as f2:
  for line in f2:
    if not re.search( ": equ ", line ):
      continue

    if line.startswith( "_" ):
      continue

    parts = re.split( ': equ ', line, 1 )

    if len( parts )<2:
      continue

    if re.search( '\.', parts[ 0 ] ):
      continue
    
    if parts[ 0 ] in exclude_list:
      continue

    parts[ 1 ] = parts[ 1 ].replace( "H", "", 1 )
 
    f1.write( "#define " + parts[ 0 ] + ' 0x' + parts[ 1 ] )

f2.close()
f1.close()

exit()