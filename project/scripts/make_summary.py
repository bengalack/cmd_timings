import string
import sys
import os

COL_LEN = 11
COLS_FOR_NAME = 1

EXT = '.map'
PATH = 'Debug' + os.path.sep + 'objs'
ROM_FILE = "Debug\\bin\\cmd_cpu.rom"

STACK_SIZE = "0x0100" # Check that this size works in main app
HIMEM_ADJUSTED = "0xF380" # Check main app
# HIMEM_ADJUSTED = "0xF196" # 0xF380 normally. 0xF196 on Sony HB-F500P

FILENAMES = [
				'startup'
 			]

FILENAMES_WITH_NON_SHARED_RAM = []

# -----------------------------------------------------------------------------
def parseFile( filename ):

	with open( PATH + os.path.sep + filename + EXT,'r' ) as f2:
		dic = DICTS[ filename ]

		for line1 in f2:
			if not line1.startswith( "     0000" ): 
				continue
			
			words = line1.split() # ['0000C000', 's__DATA']

			if words[ 1 ] in dic:
				dic[ words[ 1 ] ] = words[ 0 ][4:]

	f2.close()
	return

# -----------------------------------------------------------------------------
def calcRemainings( filename ):

	dic = DICTS[ filename ]

	if dic[ 'l__HEADER0' ] == None:
		dic[ 'l__HEADER0' ] = "0000"

	dic[ 'lowest_addr' ] 	= hex( int( dic[ 's__CODE' ], 16 ) - int( dic[ 'l__HEADER0' ], 16 ) )[2:].upper()
	dic[ 'code_end_addr' ] 	= hex( int( dic[ 's__GSFINAL' ], 16 ) - 1 )[2:].upper()
	dic[ 'code_home_len' ] 	= hex( int( dic[ 'l__CODE' ], 16 ) + int( dic[ 'l__HOME' ], 16 ) )[2:].upper()
	dic[ 'data_end_addr' ] 	= hex( int( dic[ 's__HEAP' ], 16 ) - 1 )[2:].upper()
	dic[ 'l__INITIALIZED' ]	= hex( int( dic[ 'l__INITIALIZED' ], 16 ) )[2:].upper()
	dic[ 'l__INITIALIZER' ]	= hex( int( dic[ 'l__INITIALIZER' ], 16 ) )[2:].upper()
	
	return

# -----------------------------------------------------------------------------
def getDecimalValue( filename, key ):

	return int( DICTS[ filename ][ key ], 16 )

# -----------------------------------------------------------------------------
def hexFormatted( intval ): # returns a string

	return "0x"+hex( intval ).upper()[2:]

# -----------------------------------------------------------------------------
def reportError( msg ):
	sys.stderr.write( "\n\033[31m" )
	sys.stderr.write( msg )
	sys.stderr.write( "\033[0m\n" )

	return

# -----------------------------------------------------------------------------

if len ( sys.argv )>1:
  print ( 'Usage: make_summary.py' )
  print ( '' )
  print ( '       Summary is written to std.out' )
  print ( '       Error is shown if mismatch is found' )
  print ( '       (wrong overlaps, initializers, wrong ram-loc/size, HTIMI-error)' )
  exit()

DICTS = {}							# a dict of dicts. one dict per map-file.

VALUE_PAIRS = {						# These are either grabbed from map-file or calculated
	'lowest_addr': 		None,		# to be calculated
	's__CODE': 			None,
	'l__CODE': 			None,
	's__GSFINAL': 		None,		# substract one to get it right
	'code_end_addr':	None,		# to be calculated
	'code_home_len':	None,		# to be calculated
	's__DATA': 			None,
	'l__HOME': 			None,
	's__HEAP': 			None,		# substract one to get it right
	'data_end_addr':	None,		# to be calculated
	'l__HEADER0': 		None,		# subract this from s__CODE to get lowest_addr
	'l__INITIALIZED': 	None,
	'l__INITIALIZER': 	None
}

COL_NAME = {
	'lowest_addr': 		"LOWEST ADDR",	 		# to be calculated
	's__CODE': 			"CODE START",
	'code_home_len':	"CODE LEN",
	'code_end_addr':	"CODE END",
	's__DATA': 			"DATA START",
	'data_end_addr': 	"DATA END",
	'l__INITIALIZED': 	"MUST-BE:0",
	'l__INITIALIZER': 	"MUST-BE:0"
}

# --------------------------------------------
# Write top header line
#
sys.stdout.write( '\n' )

for a in range( 0, COLS_FOR_NAME ):
	sys.stdout.write( ''.rjust(COL_LEN,' ') )

for col in COL_NAME:
	sys.stdout.write( COL_NAME[ col ].rjust(COL_LEN,' ') )

sys.stdout.write( "\n" )

for a in range( 0, COLS_FOR_NAME ):
	sys.stdout.write( ''.rjust(COL_LEN,' ') )

for col in COL_NAME:
	sys.stdout.write( "".rjust(COL_LEN,'=') )

sys.stdout.write( "\n" )

# --------------------------------------------
# Parse files and print
#
for filename in FILENAMES:
	dic = VALUE_PAIRS.copy()
	DICTS[ filename ] = dic
	parseFile( filename )
	calcRemainings( filename )

	sys.stdout.write( filename.ljust(COL_LEN,' ') )

	for col in COL_NAME:
		sys.stdout.write( dic[ col ].rjust(COL_LEN,' ') )

	sys.stdout.write( '\n' )

# --------------------------------------------
# Error checking
#
bErrorsFound = False

# DIFFY = getDecimalValue( "resident", "lowest_addr" ) - getDecimalValue( "startup", "data_end_addr" ) 
DIFFY = 0
if DIFFY<0:
	reportError( "		ERROR: \"Data\" overwrites \"resident\"" )
	bErrorsFound = True

DATA_START_HEX = "C000"
DATA_START = int( DATA_START_HEX, 16 )
for filename in FILENAMES:
	if filename not in FILENAMES_WITH_NON_SHARED_RAM:
		if getDecimalValue( filename, "s__DATA" )!=DATA_START:
			reportError( "		ERROR: \"%s\" does not have data at %s" % ( filename, DATA_START_HEX ) )
			bErrorsFound = True

# HTIMI = getDecimalValue( "game", "_LOC_MY_HTIMI" )
# if HTIMI!=DATA_START:
# 	HTIMI_HEX = hex( HTIMI ).upper()[2:]
# 	reportError( "		ERROR: \"LOC_MY_HTIMI\" is not at correct address: (%s vs %s)" % ( HTIMI_HEX, DATA_START_HEX ) )
# 	bErrorsFound = True

# DIFF = getDecimalValue( "orchestr", "lowest_addr" ) - getDecimalValue( "orchestr", "data_end_addr" )
# if DIFF<0:
# 	reportError( "		ERROR: \"orchestr\" overwrites shared data-area :-(" )
# 	bErrorsFound = True


modules = [ "startup" ]
CODE_END_HEX = "0x7FFF"
CODE_END = int( CODE_END_HEX, 16 )
for module in modules:
	if getDecimalValue( module, "code_end_addr" )>CODE_END:
		reportError( "		ERROR: \"%s\" ends code above page 1 :-(" % module )
		bErrorsFound = True

LOW_ADDR = int( HIMEM_ADJUSTED, 16 ) - int( STACK_SIZE, 16 )
LOW_ADDR_HEX = hexFormatted( LOW_ADDR )
# DIFF_HIGH = LOW_ADDR-getDecimalValue( "resident", "code_end_addr" )
DIFF_HIGH = 0

if DIFF_HIGH<0:
	reportError( "		ERROR: \"resident\" overwrites stack (stack size: %s, stack start: %s) :-(" % ( STACK_SIZE,LOW_ADDR_HEX ) )
	bErrorsFound = True

DATA_END = None
for filename in FILENAMES:
	if filename not in FILENAMES_WITH_NON_SHARED_RAM:
		vallie = getDecimalValue( filename, "data_end_addr" )
		if DATA_END is None:
			DATA_END = vallie
		else:
			if DATA_END!=vallie:
				reportError( "		ERROR: \"%s\" does not have data end same place as others (%s)" % ( filename, hexFormatted( vallie ) ) )
				bErrorsFound = True

for filename in FILENAMES:
	if getDecimalValue( filename, "l__INITIALIZED" )!=0:
		reportError( "		ERROR: \"%s\" seems to be using initializer" % filename )
		bErrorsFound = True

	if getDecimalValue( filename, "l__INITIALIZER" )!=0:
		reportError( "		ERROR: \"%s\" seems to be using initializer" % filename )
		bErrorsFound = True

sys.stdout.write( '\n' )
	# sys.stdout.write( "Bytes (decimal) between data and orchestr: ".ljust(56,' ') )
	# sys.stdout.write( "%s\n" % DIFF )
	# sys.stdout.write( "Bytes (decimal) between orchestr and myassert: ".ljust(56,' ') )
	# sys.stdout.write( "%s\n" % DIFFY )
 
# sys.stdout.write( "Bytes between data-end and resident: ".ljust(56,' ') )
# sys.stdout.write( "%s (%s)\n" % ( hexFormatted( DIFFY ), DIFFY ) )
# sys.stdout.write( "Bytes between resident-end and stack-low: ".ljust(56,' ') )
# sys.stdout.write( "%s (%s)\n" % ( hexFormatted( DIFF_HIGH ), DIFF_HIGH ) )

sys.stdout.write( "Stack-high addr: ".ljust(56,' ') )
sys.stdout.write( "%s\n" % HIMEM_ADJUSTED )

sys.stdout.write( "Stack-low addr: ".ljust(56,' ') )
sys.stdout.write( "%s\n" % LOW_ADDR_HEX )
sys.stdout.write( "Stack size (last check: 0x0000): ".ljust(56,' ') )
sys.stdout.write( "%s\n" % STACK_SIZE )

sys.stdout.write( '\n' )

file_stats = os.stat( ROM_FILE )
sys.stdout.write( "ROM file size: %d bytes (%.2f MB) - (%.1f 16kB segments)\n" % ( file_stats.st_size, file_stats.st_size/1024.0/1024, file_stats.st_size/0x4000 ) )

sys.stdout.flush()

if bErrorsFound:
	sys.stderr.flush()
	exit( 1 )

exit()
