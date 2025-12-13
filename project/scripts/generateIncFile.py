import re
import sys

AVOID = '/* ## IGNORE IN .inc-FILES ## */'
DEFINE = '/* ## DEFINE IN .inc-FILES ## */'

if len ( sys.argv )<3:
  print ( 'Usage: generateIncFile.py <inputfilename> <outputfilename> [-append]' )
  print ( '       Duplicate entries (in outfile) will not be written' )
  print ( '       lines ending with %s is ignored' % AVOID )
  print ( '       lines ending with %s gets .define instead of .equ' % DEFINE )
  exit()

existing_symbols = dict()

if len( sys.argv )>=4 and sys.argv[3]=='-append':
  with open( sys.argv[2], 'r' ) as f3: # fill existing_symbols
    for line in f3:
      existing_symbols[re.split( ' ', line )[0]] = 1 # format: "NAME" .equ "value"
  f3.close()

  f1 = open( sys.argv[2], 'a' )
else:
  f1 = open( sys.argv[2], 'w' )

with open( sys.argv[1], 'r' ) as f2:
  for line in f2:
    if line.strip().startswith( "//" ):
      continue

    parts = re.split( '#define\s+', line )

    if len( parts )<2:
      continue # ignore lines that are not defines

    tests = re.split( '//', parts[1].strip() )  # split into a possible "0, 1"-list or just "0" "SYM VALUE // comment"
    if len( re.split( '\s+', tests[0] ))==1:
      continue                                    # get rid of plain defines with no value

    parts[1] = re.sub( "//", ";", parts[1] )
 
    if len( parts )==1:
      continue # ignore lines that have no value (plain defines)

    if parts[-1].strip().endswith( AVOID ):
      continue

    if parts[-1].strip().endswith( DEFINE ):
      parts[-1] = parts[-1][:parts[-1].index(DEFINE)].strip()
      line = line[:line.index(DEFINE)].strip()
      # print(parts)
      bDefineInstead = True
    else:
      bDefineInstead = False
    
    if re.search( '\\*/|/\\*', line ): # Must be put as late as this, as we support the AVOID-case above
      print( "ERROR: Block comment found - not supported" )
      exit()

    newparts = re.split( '\s+', parts[1], 1 )  # split after first whitespace

    if newparts[0] not in existing_symbols: # ensure writing only once

      if bDefineInstead:
        f1.write( ".define %s /%s/\n" % (newparts[0], newparts[1]) )
      else:
        f1.write( '%s .equ %s'  % (newparts[0], newparts[1]) )

      existing_symbols[newparts[0]] = 1

f2.close()
f1.close()

exit()