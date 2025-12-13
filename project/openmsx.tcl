# source profile
# source symbols

# If there are identical symbols in the different sym-files, the last one will be the one used (ex: _API_xxx-symbols)
symbols::load Debug/objs/startup.noi


# symbols::load Debug/objs/orchestr_.sym
# adding this may mess up all the _API_xxx in orchestr

proc dihaltcallback {} {
	message "DI; HALT detected, which means a hang. You can just as well reset the machine now..." warning
	debug break
}

set di_halt_callback dihaltcallback

#set umr_callback umrcallback

proc cpuspeed {x} {
	variable z80_freq [expr 3579545 * $x]
	variable z80_freq_locked false
	variable cmdtiming broken
	set z80_freq
}

proc my_page0slot_pc {} {
#   Panasonic_FS-A1GT / Turbo R / Panasonic_FS-A1WSX / Sanyo PHC-70FD
	address_in_slot [reg pc] 3 0
}

proc my_slot_pc {} {
#   # # # NMS 8255 normal
#    address_in_slot [reg pc] 3 2

#   Panasonic_FS-A1GT / Turbo R / Panasonic_FS-A1WSX / Sanyo PHC-70FD
	address_in_slot [reg pc] 3 0

#   NMS 8255 / Panasonic_FS-A1WSX normal with 512kb
	# address_in_slot [reg pc] 1
}

proc my_page1slot_pc {} {

  # We are almost always cartridge slot 1 in page 1
	address_in_slot [reg pc] 1

  # NMS 8255 normal
  # address_in_slot [reg pc] 3 2

  # Panasonic_FS-A1GT / Turbo R
	# address_in_slot [reg pc] 1

  # NMS 8255 / Panasonic_FS-A1WSX normal with 512kb
	#address_in_slot [reg pc] 1
}


# ================================================================================
# COMMON - BEGIN
# ================================================================================


# ================================================================================
# COMMON - END
# ================================================================================


# debug set_bp -once [symbol _main] {[my_page1slot_pc]} {


# profile::section_scope_bp frame       		[symbol _loop] { [my_page1slot_pc] }



# ================================================================================
# GAME - BEGIN
# ================================================================================

# ================================================================================
# GAME - END
# ================================================================================

# }

profile_osd t
