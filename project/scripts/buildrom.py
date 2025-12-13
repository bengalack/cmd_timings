import subprocess
import os
from time import time
import datetime
import argparse

#########################################################################################
# Build a ROM file from a mix of .s and .c files (sick of bash/cmd!)
# Does currently not support building files in various directories
# Is expected to be placed in project/scripts with all scripts in this location
# Author: pal.hansen@gmail.com
#########################################################################################

parser = argparse.ArgumentParser(description='Build a ROM file from a mix of .s and .c files. CRT-file MUST be first, if present. Last input filename is used for output name.')

parser.add_argument('-O','--optimize', help='speed a lot of time, making file a tad faster & smaller', action='store_true', default=False, required=False)
parser.add_argument('-X','--debug',    help='pass on debug to compiler, generates cdb files and lots of symbols', action='store_true', default=False, required=False)
parser.add_argument('code_loc',        help='code location in hex (0x)')
parser.add_argument('data_loc',        help='data location in hex (0x)')
parser.add_argument('work_dir',        help='dir relative to git-root')
parser.add_argument('filename',        help='a list of filenames', nargs='+', default=[])

args = vars(parser.parse_args())
CODE_LOC    = args['code_loc']
DATA_LOC    = args['data_loc']
WORK_DIR    = args['work_dir']
FILES       = args['filename']

#To generate structs.inc we need "--debug"
if args['debug']:
    DEBUG = "--debug"
else:
    DEBUG = ""


if args['optimize']:
    OPT = "--opt-code-speed --max-allocs-per-node 2000000" # Ingame takes 10:53 on this one (saves around 0x120 bytes)
    # OPT = "--opt-code-speed --max-allocs-per-node 500000" # Ingame takes 4:11 on this one, NO REAL OPT! = WASTE
else:
    OPT = "--opt-code-speed" # 0x7AD3

OUTPUT_EXTENSION="rom"
SRC_ROOT = "src"
PROFILE = "Debug"

MSX_OBJ_PATH = PROFILE + os.sep + "objs"
MSX_BIN_PATH = PROFILE + os.sep + "bin"
FLAGS = "-o -g -s -p -w -I" + SRC_ROOT

#########################################################################################
VERBOSE = True
MSX_FILE_NAME = FILES[-1]
objectlist = []

# =====================================
def printNewBlock(s):
    print("-------------------------------------------------")
    print(s)
    
# =====================================
def cutExtension(s):
    pos = s.rfind('.')
    if pos>=0:
        return s[0:s.rfind('.')] # cut the ext
    else:
        return s

# =====================================
def executeCmd(s):
    if VERBOSE:
        print(s)
    subprocess.run(s, shell=True, check=True)

# =====================================
def createRel(fullname):

    rel = MSX_OBJ_PATH + os.sep + cutExtension(fullname) + ".rel"
    src = SRC_ROOT + os.sep + fullname

    if fullname.endswith('.c'):
        s = "sdcc --less-pedantic %s -mz80 %s -c -I%s -Wa -I%s -o %s %s" % (DEBUG, OPT, SRC_ROOT, SRC_ROOT, MSX_OBJ_PATH+os.sep, src)
        # s = "sdcc --verbose --debug -mz80 %s -c -I%s -Wa -I%s -o %s %s" % (OPT, SRC_ROOT, SRC_ROOT, MSX_OBJ_PATH+os.sep, src)
    else:
        s = "sdasz80 %s %s %s" % (FLAGS, rel, src)

    executeCmd(s)

    return rel

# -----------------------------------------------------------------------------
# looks for l__HEADER0 - only present in files that stem from CRT-files
def parseSymFileGetOffset(filename):

    with open(filename,'r') as f2:
        for line1 in f2:
            if not line1.startswith("  "): 
                continue
            
            words = line1.split() # 2 _HEADER0  size     13   flags    8

            if words[1] == "_HEADER0":
                return int(words[3], 16)

    f2.close()
    return 0

# =====================================
# Use CODE_LOC as base, but check the first rel file (which should be the CRT-file, if it has a size for "_HEADER0")
# Looks like this: A _HEADER0 size 13 flags 8 addr 4000
#
def findCorrectCodeLoc(sym_file, code_loc_base_str):
    
    offset = parseSymFileGetOffset(sym_file)

    if offset==0:
        return code_loc_base_str
    
    code_loc_base_int = int(code_loc_base_str, 16) # it is assumed that this string has 0x prefix
    new_code_loc = code_loc_base_int + offset

    return hex(new_code_loc)

# =====================================
def linkRelsAndMakeRom(obj_arr, rom_name):
    
    core_name = MSX_OBJ_PATH + os.sep + rom_name
    ihx = core_name + ".ihx"

    objs = " ".join(obj_arr)

    new_code_loc = findCorrectCodeLoc(MSX_OBJ_PATH + os.sep + cutExtension(FILES[0]) + ".sym", CODE_LOC)

    # s = "sdcc --less-pedantic --code-loc %s --data-loc %s -mz80 --no-std-crt0 %s --disable-warning 196 %s -o %s" % (new_code_loc, DATA_LOC, OPT, objs, ihx)
    s = "sdcc %s --code-loc %s --data-loc %s -mz80 --no-std-crt0 %s --disable-warning 196 %s -o %s" % (DEBUG, new_code_loc, DATA_LOC, OPT, objs, ihx)
    # s = "sdcc --verbose --code-loc %s --data-loc %s -mz80 --no-std-crt0 %s --disable-warning 196 %s -o %s" % (new_code_loc, DATA_LOC, OPT, objs, ihx)
    executeCmd(s)

    s = "hex2bin -e %s %s" % (OUTPUT_EXTENSION, ihx)
    executeCmd(s)

# =====================================
def mil_convert(milliseconds):
   seconds, milliseconds = divmod(milliseconds, 1000)
   minutes, seconds = divmod(seconds, 60)
   return minutes, seconds

# =====================================
def doTiming(stamp1):

    stamp2 = int(time()*1000)
    milliseconds = stamp2-stamp1 
    minutes, seconds = mil_convert(milliseconds)
    return f"{str(minutes).rjust(2,'0')}:{str(seconds).rjust(2,'0')} ({milliseconds} ms)"

# =====================================
def ensureRightWorkingDir(): # normally we are in either root, or in project/. we should be in project
    
    cur = os.getcwd()
    if not cur.endswith(WORK_DIR):
        os.chdir(cur + os.sep + WORK_DIR)
 
# =====================================
    
def main():
    stamp1 = int(time()*1000)
    
    ensureRightWorkingDir()
    
    printNewBlock("Deleting old .rel-files")
    cmd = "del %s*.rel" % (MSX_OBJ_PATH + os.sep)
    executeCmd(cmd)

    printNewBlock("Building ...")
    for file in FILES:
        objectlist.append(createRel(file))

    printNewBlock("Build main and link + build rom")
    linkRelsAndMakeRom(objectlist, cutExtension(MSX_FILE_NAME))

    printNewBlock("Last address:")
    noi = "." + os.sep + MSX_OBJ_PATH + os.sep + cutExtension(MSX_FILE_NAME) + ".noi"
    executeCmd("powershell -command \"gc '%s' | ? { $_.Trim() -and $_ -match 's__HEAP'}\"" % noi)

    # printNewBlock("Make symbol file...")
    printNewBlock(".noi content cleanup > noi.noi...")
    symfilepath = MSX_OBJ_PATH+os.sep
    symfilename = cutExtension(MSX_FILE_NAME)    # NOTE: without ext

    executeCmd("powershell -Command \"Get-Content '%s%s.noi' | Where-Object { $_ -notmatch '.__.' -and $_ -notmatch 's__' -and $_ -notmatch 'l__' -and $_ -notmatch '\$' -and $_ -notlike 'LOAD*' }\" > %s%s.noi.noi" % (symfilepath, symfilename, symfilepath, symfilename))

    # export a file with specific values, like length of ROM/file
    printNewBlock("Generate .h file for this rom holding size info")
    map_input = symfilepath+symfilename+".map"
    h_output = "src"+os.sep+"include_gen"+os.sep+symfilename+".h"
    executeCmd("python ./scripts/extractMapValue.py %s %s l__CODE" % (map_input, h_output))

    printNewBlock("Time spent: %s" % doTiming(stamp1))
    print("Build ended: %s" % datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'))

main()

