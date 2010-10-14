#! /usr/bin/env wish

#
# Simple GUI for Monte
#
# (C) Jiri Brozovsky, 2006
#

package require Tk

#### PATHS AND EXECUTABLES (edit them to fit your needs...)
set homedir $env(HOME)
set bin_dir "$homedir/monte" ;# directory where histograms are stored 
set his_dir "$bin_dir/histograms" ;# directory where histograms are stored 

set monte_exe "$bin_dir/monte" ;# Monte binary
set molitest_exe "$bin_dir/molitest" ;# library tester binary
set dis2gp "$bin_dir/dis2gp" ;# .dis to gnuplot converter
set dis2tk "$bin_dir/dis2tcl" ;# .dis to Tk converter
set runmonte "/tmp/runmonte-[pid]" ;# temporary file for Monter run script

set def_viewer "gvim" ;# text viewer binary

set o_libend ".so" ;# depends on operating system (.so .dll .dylib)

# Initial values of variables:
set we_have_results 0

#
# Please DO NOT EDIT the stuff below this line # ##########################


#### FILES:

set i_file_name "" ;#"default.mon"
set o_stat_name "" ;#"default.stt"
set o_sim_name  "" ;#"default.sim"
set o_h_f       "" ;#"default-"
set i_library   ""
set i_libarg    ""

#### SIMULATION 

set sim_number  10000
set sim_verbose 1000
set sim_test    1000
set sim_wall    5
set sim_savesim 0

#### DATA STRUCTURES:

# Input data:
set i_len 0
set i_name [list]
set i_mult [list]
set i_type [list]
set i_his  [list]

# Output data:
set o_len 0
set o_name [list]
set o_type [list]
set o_int  [list]
set o_from [list]
set o_to   [list]

# Results:
set rs_len  0
set rs_sims 0
set rs_min   [list]
set rs_max   [list]
set rs_aver  [list]
set rs_disp  [list]
set rs_stdev [list]
set rs_varc  [list]

# Correlations:
set corr_size 0
set corr_list [list]
set corr_val [list]

#### GUI variables:

set varname   ""
set cmult     1.0
set ivtype    "const"
set hisname   ""
set use_windowed_mode 0

set ovname ""
set ovtype 0
set ovnum  ""
set ovfrom ""
set ovto   ""

set rlabel1 ""
set rlabel2 ""


#### PROCEDURES:

proc clean_all_data {} {
  global i_len i_name i_mult i_type i_his  
  global o_len o_name o_type o_int  o_from o_to   

  unset i_name 
  unset i_mult 
  unset i_type 
  unset i_his  
  unset o_name 
  unset o_type 
  unset o_int  
  unset o_from 
  unset o_to   

  set i_len 0
  set i_name [list]
  set i_mult [list]
  set i_type [list]
  set i_his  [list]

  set o_len 0
  set o_name [list]
  set o_type [list]
  set o_int  [list]
  set o_from [list]
  set o_to   [list]
}

# adds new input variable
proc new_i {} {
  global varname cmult ivtype hisname
  global i_name i_mult i_type  i_his i_len

	clean_results
	clean_canvas

  if {[string length $varname] > 0} {
    if {$ivtype == "his"} {
      if {[string length $hisname] < 1} {
        # histogram have to be specified
        tk_messageBox -icon error -type ok -message "Histogram file must be specified"
        return -1 ;
      }
    }

    # little cleaning (spaces) and setting upper letters
    set varname [string trim $varname]
    set varname [string toupper $varname]

    # not already used?
    if {[llength $i_name] > 0} {
      if {[lsearch $i_name $varname] >= 0} {
        # must not be previously used!
        tk_messageBox -icon error -type ok -message "The given variable name already exists!"
        return -1 ;
      }
    }

    set cmult [format "%f" $cmult]

    # add input data:
    lappend i_name $varname
    lappend i_mult $cmult
    lappend i_type $ivtype
    if {$ivtype == "his"} {
      lappend i_his $hisname
    } else {
      lappend i_his ""
    }
    

    #.iv.ivlist insert end $varname
    .iv.ivlist insert end [concat $varname  " * "  $cmult  " *   <" [lindex $i_his $i_len] ">"]

    incr i_len

    set varname ""
    set cmult 1.0
    set ivtype "const"
    set hisname ""

  } else {
    # at least something is required!
    tk_messageBox -icon error -type ok -message "Invalid or empty variable name!"
    return -1 ;
  }
}

# mouse selection in .iv.ivlist
proc refresh_i_list {} {
  global varname cmult ivtype hisname
  global i_name i_mult i_type  i_his i_len
  global .iv.ivlist

	clean_canvas ; hide_results

  if {$i_len > 0} {
    # variable name string separation:
    set test_str0 [.iv.ivlist get [.iv.ivlist curselection]]
    set s_end [expr [string first "*" $test_str0 ] - 1 ]
    set test_str1 [string range $test_str0 0 $s_end]
    set test_str [string trim $test_str1]

    set pos [lsearch $i_name $test_str]

    set varname [lindex $i_name $pos]
    set cmult [lindex $i_mult $pos]
    set ivtype [lindex $i_type $pos]
    set hisname [lindex $i_his $pos]

    if {$ivtype == "his"} {
      show_i_dis
    } else {
      clean_canvas
    }

  }
}

# edit existing input variable
proc edit_i {} {
  global varname cmult ivtype hisname
  global i_name i_mult i_type  i_his i_len

	clean_results
	clean_canvas

  if {[string length $varname] > 0} {
    if {$ivtype == "his"} {
      if {[string length $hisname] < 1} {
        # histogram have to be specified
        tk_messageBox -icon error -type ok -message "Histogram file must be specified"
        return -1 ;
      }
    }

    # little cleaning (spaces) and setting upper letters
    set varname [string trim $varname]
    set varname [string toupper $varname]

    # not already used?
    if {[llength $i_name] > 0} {
      set pos [lsearch $i_name $varname]
      if { $pos >= 0} {
        # OK!
      } else {
        tk_messageBox -icon error -type ok -message "Nothing to change!"
      return -1 ;
      }
    } else {
        tk_messageBox -icon error -type ok -message "Nothing to change!"
      return -1 ;
    }


    # add input data:
    set i_mult [lreplace $i_mult $pos $pos $cmult]
    set i_type [lreplace $i_type $pos $pos $ivtype]
    if {$ivtype == "his"} {
      set i_his [lreplace $i_his $pos $pos $hisname]
    } else {
      set i_his [lreplace $i_his $pos $pos ""]
    }
    

    #.iv.ivlist insert end $varname
    .iv.ivlist delete $pos 
    .iv.ivlist insert [expr $pos ] [concat $varname  " * "  $cmult  " *   <" [lindex $i_his  $pos ] ">"]

    set varname ""
    set cmult 1.0
    set ivtype "const"
    set hisname ""

  } else {
    # at least something is required!
    tk_messageBox -icon error -type ok -message "Invalid or empty variable name!"
    return -1 ;
  }
}


# delete existing input variable
proc delete_i {} {
  global varname cmult ivtype hisname
  global i_name i_mult i_type  i_his i_len

  clean_results; clean_canvas

  if {[string length $varname] > 0} {
    
    # little cleaning (spaces) and setting upper letters
    set varname [string trim $varname]
    set varname [string toupper $varname]

    # not already used?
    if {[llength $i_name] > 0} {
      set pos [lsearch $i_name $varname]
      if { $pos >= 0} {
        # OK!
      }
    } else {
        tk_messageBox -icon error -type ok -message "Nothing to delete!"
      return -1 ;
    }


    # delete stuff (not a straightforward way :-/ ): 

    set i_name0 [list]
    set i_mult0 [list]
    set i_type0 [list]
    set i_his0 [list]

    
    for {set i 0} {$i < [expr $i_len]} {incr i} {
      if {$i != $pos} {
        lappend i_name0 [lindex $i_name $i] 
        lappend i_mult0 [lindex $i_mult $i] 
        lappend i_type0 [lindex $i_type $i] 
        lappend i_his0 [lindex $i_his $i] 
      }
    }

    unset i_name 
    unset i_mult
    unset i_type 
    unset i_his 

    set i_name $i_name0 
    set i_mult $i_mult0 
    set i_type $i_type0 
    set i_his  $i_his0 

    incr i_len -1


    .iv.ivlist delete $pos 

    set varname ""
    set cmult 1.0
    set ivtype "const"
    set hisname ""

  } else {
    # at least something is required!
    tk_messageBox -icon error -type ok -message "Nothing to delete!"
    return -1 ;
  }
}

proc show_i_dis {} {
  global hisname dis2tk

  if {$hisname == ""} {
    tk_messageBox -icon error -type ok -message "Histogram file must be specified"
    return -1;
  } else {
    exec $dis2tk $hisname
    set name $hisname
    append name ".tk"
    source  $name
    gnuplot .sv.c
  }
}


# mouse selection in .ov.ovlist
proc refresh_o_list {} {
  global ovname ovtype ovnum ovfrom ovto
  global o_len o_name o_type o_int o_from o_to
  global .ov.ovlist
	global we_have_results

  if {$o_len > 0} {
    # variable name string separation:
    set test_str0 [.ov.ovlist get [.ov.ovlist curselection]]

    set s_end [expr [string first "*" $test_str0 ] - 1 ]
    set test_str1 [string range $test_str0 0 $s_end]
    set test_str [string trim $test_str1]

    set pos [lsearch $o_name $test_str]

    set ovname [lindex $o_name $pos]
    set ovtype [lindex $o_type $pos]
    set ovnum [lindex $o_int $pos]
    set ovfrom [lindex $o_from $pos]
    set ovto [lindex $o_to $pos]

    if {$ovtype > 0} {
      plot_result_hist
    } else {
      clean_canvas
    }

		show_result $pos

		if {$we_have_results == 0} {clean_canvas}
  }
}

# adds new output variable
proc new_o {} {
  global ovname ovtype ovnum ovfrom ovto
  global o_len o_name o_type o_int o_from o_to
  global .ov.ovlist

  clean_results; clean_canvas

  if {[string length $ovname] > 0} {

    # little cleaning (spaces) and setting upper letters
    set ovname [string trim $ovname]
    set ovname [string toupper $ovname]

    # not already used?
    if {[llength $o_name] > 0} {
      if {[lsearch $o_name $ovname] >= 0} {
        # must not be previously used!
        tk_messageBox -icon error -type ok -message "The given variable name already exists!"
        return -1 ;
      }
    }

    # check histogram type
    set o_ok 0

    if {$ovtype == 0} {
      set ovnum ""
      set ovfrom ""
      set ovto ""

      set ot_str "unused"
      set o_ok 1
    }

    if {$ovtype == 1} {
      set ovnum ""
      set ovfrom ""
      set ovto ""

      set ot_str "automatic"
      set o_ok 1
    }

    if {$ovtype == 2} {
      set ovnum ""
      set ovfrom ""
      set ovto ""

      set ot_str "automatic, dynamic"
      set o_ok 1
    }

    if {$ovtype == 3} {
      if {$ovnum < 1} {
      tk_messageBox -icon error -type ok -message "Number of intervals have to be > 1"
        return -1 ;
      }
      set ovnum [expr round($ovnum)]
      set ovfrom ""
      set ovto ""

      set ot_str $ovnum
      set o_ok 1
    }
    
    if {$ovtype == 4} {
      if {$ovnum < 1} {
        tk_messageBox -icon error -type ok -message "Number of intervals have to be > 1"
        return -1 ;
      }
      set ovnum [expr round($ovnum)]
      set ovfrom ""
      set ovto ""

      set ot_str [concat $ovnum " dynamic " ]
      set o_ok 1
    }
    
    if {$ovtype == 5} {
      if {$ovnum < 1} {
        tk_messageBox -icon error -type ok -message "Number of intervals have to be > 1"
        return -1 ;
      }
      if {$ovfrom == ""} { tk_messageBox -icon error -type ok -message "Invalid limits!" ; return -1 ; }
      if {$ovto == ""} { tk_messageBox -icon error -type ok -message "Invalid limits!" ; return -1 ; }
      if {$ovfrom >= $ovto} {
        tk_messageBox -icon error -type ok -message "Invalid limits!"
        return -1 ;
      }

      set ovnum [expr round($ovnum)]

      set ot_str [concat $ovnum "<" $ovfrom ";" $ovto ">"]
      set o_ok 1
    }
 
    if {$ovtype == 6} {
      if {$ovnum < 1} {
        tk_messageBox -icon error -type ok -message "Number of intervals have to be > 1"
        return -1 ;
      }
      if {[ string compare $ovfrom ""] == 1} { tk_messageBox -icon error -type ok -message "Invalid limits!" return -1 ; }
      if {[ string compare $ovto ""] == 1} { tk_messageBox -icon error -type ok -message "Invalid limits!" return -1 ; }
      if {$ovfrom >= $ovto} {
        tk_messageBox -icon error -type ok -message "Invalid limits!"
        return -1 ;
      }

      set ovnum [expr round($ovnum)]

      set ot_str [concat $ovnum "<" $ovfrom ";" $ovto ">, dynamic "]
      set o_ok 1
    }



    if {$o_ok == 0} {
      tk_messageBox -icon error -type ok -message "Variable type michmach!"
      return -1 ;
    }

    # add input data:
    lappend o_name $ovname
    lappend o_type $ovtype
    lappend o_int $ovnum
    lappend o_from $ovfrom
    lappend o_to $ovto


    #.iv.ivlist insert end $varname
    .ov.ovlist insert end [concat $ovname  " * "  $ot_str ]

    incr o_len

    # clean stuff
    set ovname ""
    set ovtype 0
    set ovnum ""
    set ovfrom ""
    set ovto ""

  } else {
    # at least something is required!
    tk_messageBox -icon error -type ok -message "Invalid or empty variable name!"
    return -1 ;
  }
}


# edit output variable
proc edit_o {} {
  global ovname ovtype ovnum ovfrom ovto
  global o_len o_name o_type o_int o_from o_to
  global .ov.ovlist

  clean_results; clean_canvas

  if {[string length $ovname] > 0} {

    # little cleaning (spaces) and setting upper letters
    set ovname [string trim $ovname]
    set ovname [string toupper $ovname]

    # not already used?
    if {[llength $o_name] > 0} {
      if {[set pos [lsearch $o_name $ovname]] < 0} {
        # must not be previously used!
        tk_messageBox -icon error -type ok -message "The variable must already exists!"
        return -1 ;
      }
    } else {
      tk_messageBox -icon error -type ok -message "Nothing to edit!"
      return -1 ;
    }

    # check histogram type
    set o_ok 0

    if {$ovtype == 0} {
      set ovnum ""
      set ovfrom ""
      set ovto ""

      set ot_str "unused"
      set o_ok 1
    }

    if {$ovtype == 1} {
      set ovnum ""
      set ovfrom ""
      set ovto ""

      set ot_str "automatic"
      set o_ok 1
    }

    if {$ovtype == 2} {
      set ovnum ""
      set ovfrom ""
      set ovto ""

      set ot_str "automatic, dynamic"
      set o_ok 1
    }

    if {$ovtype == 3} {
      if {$ovnum < 1} {
      tk_messageBox -icon error -type ok -message "Number of intervals have to be > 1"
        return -1 ;
      }
      set ovnum [expr round($ovnum)]
      set ovfrom ""
      set ovto ""

      set ot_str $ovnum
      set o_ok 1
    }
    
    if {$ovtype == 4} {
      if {$ovnum < 1} {
        tk_messageBox -icon error -type ok -message "Number of intervals have to be > 1"
        return -1 ;
      }
      set ovnum [expr round($ovnum)]
      set ovfrom ""
      set ovto ""

      set ot_str [concat $ovnum " dynamic " ]
      set o_ok 1
    }
    
    if {$ovtype == 5} {
      if {$ovnum < 1} {
        tk_messageBox -icon error -type ok -message "Number of intervals have to be > 1"
        return -1 ;
      }
      if {$ovfrom >= $ovto} {
        tk_messageBox -icon error -type ok -message "Invalid limits!"
        return -1 ;
      }

      set ovnum [expr round($ovnum)]

      set ot_str [concat $ovnum "<" $ovfrom ";" $ovto ">"]
      set o_ok 1
    }
 
    if {$ovtype == 6} {
      if {$ovnum < 1} {
        tk_messageBox -icon error -type ok -message "Number of intervals have to be > 1"
        return -1 ;
      }
      if {$ovfrom >= $ovto} {
        tk_messageBox -icon error -type ok -message "Invalid limits!"
        return -1 ;
      }

      set ovnum [expr round($ovnum)]

      set ot_str [concat $ovnum "<" $ovfrom ";" $ovto ">, dynamic "]
      set o_ok 1
    }

    if {$o_ok == 0} {
      tk_messageBox -icon error -type ok -message "Variable type michmach!"
      return -1 ;
    }

    # add input data:
    set o_type [lreplace $o_type $pos $pos $ovtype]
    set o_int [lreplace $o_int $pos $pos $ovnum]
    set o_from [lreplace $o_from $pos $pos $ovfrom]
    set o_to [lreplace $o_to $pos $pos $ovto]


    #.iv.ivlist insert end $varname
    .ov.ovlist delete $pos 
    .ov.ovlist insert [expr $pos] [concat $ovname  " * "  $ot_str ]

    # clean stuff
    set ovname ""
    set ovtype 0
    set ovnum ""
    set ovfrom ""
    set ovto ""

  } else {
    # at least something is required!
    tk_messageBox -icon error -type ok -message "Invalid or empty variable name given!"
    return -1 ;
  }
}

# delete existing input variable
proc delete_o {} {
  global ovname ovtype ovnum ovfrom ovto
  global o_len o_name o_type o_int o_from o_to
  global .ov.ovlist

  clean_results; clean_canvas

  if {[string length $ovname] > 0} {

    # little cleaning (spaces) and setting upper letters
    set ovname [string trim $ovname]
    set ovname [string toupper $ovname]

    # not already used?
    if {[llength $o_name] > 0} {
      if {[set pos [lsearch $o_name $ovname]] < 0} {
        # must not be previously used!
        tk_messageBox -icon error -type ok -message "The variable must already exists!"
        return -1 ;
      }
    } else {
      tk_messageBox -icon error -type ok -message "Nothing to delete!"
      return -1 ;
    }

    # delete stuff (not a straightforward way :-/ ): 

    set o_name0 [list]
    set o_type0 [list]
    set o_int0 [list]
    set o_from0 [list]
    set o_to0 [list]

    
    for {set i 0} {$i < [expr $o_len]} {incr i} {
      if {$i != $pos} {
        lappend o_name0 [lindex $o_name $i] 
        lappend o_type0 [lindex $o_type $i] 
        lappend o_int0 [lindex $o_int $i] 
        lappend o_from0 [lindex $o_from $i] 
        lappend o_to0 [lindex $o_to $i] 
      }
    }

    unset o_name 
    unset o_type 
    unset o_int 
    unset o_from
    unset o_to 

    set o_name $o_name0 
    set o_type $o_type0 
    set o_int $o_int0 
    set o_from $o_from0 
    set o_to $o_to0 

    incr o_len -1


    .ov.ovlist delete $pos 

    # clean stuff
    set ovname ""
    set ovtype 0
    set ovnum ""
    set ovfrom ""
    set ovto ""

  } else {
    # at least something is required!
    tk_messageBox -icon error -type ok -message "Nothing to delete!"
    return -1 ;
  }
}

proc set_file_names {} {
  global i_file_name o_stat_name o_sim_name o_h_f

  if {[string length $i_file_name] > 0 } {
    set o_stat_name ""
    set o_sim_name ""
    set o_h_f ""

    append o_stat_name  [file rootname $i_file_name] ".stt"
    append o_sim_name  [file rootname $i_file_name] ".sim"
    append o_h_f  [file rootname $i_file_name] "-"

    #puts $i_file_name
    #puts $o_sim_name
    #puts $o_stat_name
    #puts $o_h_f
  }
}

# writes input data to file:
proc write_input_file {} {
  global i_file_name
  global i_name i_mult i_type  i_his i_len
  global o_len o_name o_type o_int o_from o_to
  global corr_size corr_list corr_val

  if {$i_file_name == ""} {
    tk_messageBox -icon error -type ok -message "Need to set name of input file!"
    return -1
  }

  # open file:
  if [ catch {set fr [open $i_file_name "w"]} result] {
    #puts $result  
    tk_messageBox -icon error -type ok -message "Can not open file!"
    return -1 
  }

  puts $fr $i_len
  if {$i_len > 0} {
    for {set i 0} {$i< $i_len} {incr i} {
      puts -nonewline $fr [ lindex $i_name $i] ; puts -nonewline $fr " "
      puts -nonewline $fr [ lindex $i_mult $i] ; puts -nonewline $fr " "
      
      if {[string compare [lindex $i_type $i] "const"] == 0 } {
        set itype 0
      } else {
        if {[string compare [lindex $i_type $i] "hcopy"] == 0 } {
          set itype 2
        } else {
          # "his"
          set itype 1
        }
      }
      
      puts -nonewline $fr $itype
      puts -nonewline $fr " "
      puts -nonewline $fr [ lindex $i_his $i]
      puts $fr " "
    }
  }

  puts $fr $o_len
  if {$o_len > 0} {
    for {set i 0} {$i< $o_len} {incr i} {
      puts -nonewline $fr [ lindex $o_name $i] ; puts -nonewline $fr " "
      puts -nonewline $fr [ lindex $o_type $i] ; puts -nonewline $fr " "
      puts -nonewline $fr [ lindex $o_int $i] ; puts -nonewline $fr " "
      puts -nonewline $fr [ lindex $o_from $i] ; puts -nonewline $fr " "
      puts -nonewline $fr [ lindex $o_to $i]
      puts $fr " "
    }
  }

 # correlations:
  puts $fr $corr_size
  if {$corr_size > 0} {
    for {set i 0} {$i< $corr_size} {incr i} {
      puts $fr [lindex $corr_list $i]
    }

    set num 0

    for {set i 0} {$i < [expr $corr_size]} {incr i} {
      for {set j 0} {$j < [expr $corr_size]} {incr j} {
        puts -nonewline $fr [ lindex $corr_val $num]
        puts -nonewline $fr " "
        incr num
      }
      puts $fr ""
    }
  }


       
  # close file:
  if [ catch [close $fr] result] { 
    tk_messageBox -icon error -type ok -message "Can not close file!"
    #puts $result 
    return -1 
  }


  wm title . [concat "Monte \[" $i_file_name "\]"]
  set_file_names

  return 0;
}

# reads input data from file:
proc read_input_file {} {
  global i_file_name
  global i_name i_mult i_type  i_his i_len
  global o_len o_name o_type o_int o_from o_to
  global corr_size corr_list corr_val

	clean_results
  clean_canvas

  if {$i_file_name == ""} {
    tk_messageBox -icon error -type ok -message "Need to set name of input file!"
  }


  # open file:
  if [ catch {set fr [open $i_file_name "r"]} result] {
    #puts $result  
    tk_messageBox -icon error -type ok -message "Can not open file!"
    return -1 
  }

  .iv.ivlist delete 0 $i_len
  .ov.ovlist delete 0 $o_len

  unset i_mult
  unset i_type
  unset i_his
  unset i_name

  set i_name [list]
  set i_mult [list]
  set i_type [list]
  set i_his [list]

  unset o_name
  unset o_type
  unset o_int
  unset o_from
  unset o_to

  set o_name [list]
  set o_type [list]
  set o_int [list]
  set o_from [list]
  set o_to [list]

  set line [list]

  gets $fr i_len
  
  if {$i_len > 0} {
    for {set i 0} {$i< $i_len} {incr i} {
      gets $fr line
      set name [lindex $line 0]
      set mult [lindex $line 1]
      set type [lindex $line 2]

      if {$type == 1} {
        # read hname
        set hname [lindex $line 3]
        set stype "his"
      } else {
        set hname ""
        set stype "const"
      }
      lappend i_name $name
      lappend i_mult $mult
      lappend i_type $stype
      lappend i_his $hname

      .iv.ivlist insert end [concat $name  " * "  $mult  " *   <" $hname ">"]
    } 
  }


  # output data:
  set line [list]

  gets $fr o_len
  if {$o_len > 0} {
    for {set i 0} {$i< $o_len} {incr i} {
      gets $fr line
      
      set name [lindex $line 0]
      set type [lindex $line 1]

      set ot_str ""

      if {$type == 0} {
        set int ""
        set from ""
        set to ""
        set ot_str "unused"
      }

      if {$type == 1} {
        set int ""
        set from ""
        set to ""
        set ot_str "automatic"
      }

      if {$type == 2} {
        set int ""
        set from ""
        set to ""
        set ot_str "automatic, dynamic"
      }

      if {$type == 3} {
        set int [lindex $line 2]
        set from ""
        set to ""
        set ot_str $int
      }

      if {$type == 4} {
        set int [lindex $line 2]
        set from ""
        set to ""
        set ot_str [concat $int " dynamic " ]
      }
      
        if {$type == 5} {
        set int [lindex $line 2]
        set from   [lindex $line 3]
        set to   [lindex $line 4]
        set ot_str [concat $int "<" $from ";" $to ">"]
      }
 
        if {$type == 6} {
        set int [lindex $line 2]
        set from   [lindex $line 3]
        set to   [lindex $line 4]
        set ot_str [concat $int "<" $from ";" $to ">, dynamic "]
      }

      lappend o_name $name
      lappend o_type $type
      lappend o_int $int
      lappend o_from $from
      lappend o_to $to

    .ov.ovlist insert end [concat $name  " * "  $ot_str ]
    }
  }

  # correlations:
  unset corr_list
  unset corr_val

  set corr_size 0
  set corr_list [list]
  set corr_val  [list]

  gets $fr corr_size

  if {$corr_size > 0} {
    set varname ""
    for {set i 0} {$i < [expr $corr_size]} {incr i} {
      gets $fr line
      set varname [lindex $line 0]
      lappend corr_list $varname
    }

    for {set i 0} {$i < [expr $corr_size]} {incr i} {
      gets $fr line
      for {set j 0} {$j < [expr $corr_size]} {incr j} {
      set varname [lindex $line [expr $j]]
      lappend corr_val $varname
      }
    }
  }

  # close file:
  if [ catch [close $fr] result] { 
    tk_messageBox -icon error -type ok -message "Can not close file!"
    #puts $result 
    return -1 
  }

  wm title . [concat "Monte \[" $i_file_name "\]"]
  set_file_names

  return 0;
}

# creates a epty file
proc fileNew {} {
  global we_have_results i_len o_len
  global i_mult i_type i_his i_name
  global o_name o_type o_int o_from o_to
  global i_file_name o_stat_name o_sim_name
  global o_h_f i_library i_libarg    
  global .iv.ivlist .ov.ovlist
  global i_libarg i_library
  global corr_size corr_list corr_val

  # GUI cleaning:
  clean_canvas
	clean_results
  set i_library ""
  set i_libarg ""

  if {$i_len > 0} { .iv.ivlist delete 0 [expr $i_len - 1] }
  if {$o_len > 0} { .ov.ovlist delete 0 [expr $i_len - 1] }

  # data cleaning:
  set we_have_results 0

  set i_len 0
  set o_len 0

  unset i_mult
  unset i_type
  unset i_his
  unset i_name

  set i_name [list]
  set i_mult [list]
  set i_type [list]
  set i_his [list]

  unset o_name
  unset o_type
  unset o_int
  unset o_from
  unset o_to

  set o_name [list]
  set o_type [list]
  set o_int [list]
  set o_from [list]
  set o_to [list]

  set line [list]

  unset corr_list
  unset corr_val
  set corr_size 0
  set corr_list [list]
  set corr_val  [list]

  set i_file_name "" ;#"default.mon"
  set o_stat_name "" ;#"default.stt"
  set o_sim_name  "" ;#"default.sim"
  set o_h_f       "" ;#"default-"
  set i_library   ""
  set i_libarg    ""
}

# reads from file that is selected by dialog
proc fileOpen {} {
  global i_file_name
  global i_libarg i_library
  global .iv.ivlist .ov.ovlist
  global i_libarg i_library

  # opening:
  set name ""
  
  set types { 
    {{Monte input files}       {.mon}        TEXT} 
    {{Monte input files}       {.MON}        TEXT} 
  }

  set name [tk_getOpenFile -defaultextension  ".mon" -filetypes $types]

  if {$name == ""} { return } 

  # cleaning of old library:
  set i_library ""
  set i_libarg ""

  set i_file_name $name

  if {[read_input_file] != 0} {
    set i_file_name ""
    clean_all_data
    return -1
  }
}

# saves to file that is selected by dialog
proc fileSaveAs {} {
  global i_file_name

  set name ""
  
  set types { 
    {{Monte input files}       {.mon}        TEXT} 
    {{Monte input files}       {.MON}        TEXT} 
  }

  set name [tk_getSaveFile -defaultextension  ".mon" -filetypes $types]
  
  if {$name == ""} { return } 

  set i_file_name $name

  if {[write_input_file] != 0} {
    set i_file_name ""
    return -1
  }
}

# reads from file that is selected by dialog
proc setLib {} {
  global o_libend i_library

  set name ""
  
  set types { 
    {{Dynamic library}       {.so}        } 
    {{Dynamic library}       {.dylib}        } 
    {{Dynamic library}       {.dll}        } 
  }

  set name [tk_getOpenFile -defaultextension  $o_libend -filetypes $types]

  if {$name == ""} { return } 
  set i_library $name
}


# reads from file that is selected by dialog
proc setLibArg {} {
  global i_libarg

  set name ""
  
  set types { 
    {{Text Files}       {.txt}        TEXT} 
    {{Text Files}       {.TXT}        TEXT} 
    {{uFEM Files}       {.fem}        TEXT} 
    {{All files}       {*}        TEXT} 
  }

  set name [tk_getOpenFile -filetypes $types]

  if {$name == ""} { return } 
  set i_libarg $name
}

# executes solver:
proc simRun {real_run} {
  global sim_number  sim_verbose sim_test  sim_wall  sim_savesim 
  global monte_exe runmonte
  global i_file_name o_stat_name o_sim_name  o_h_f i_library i_libarg
  global we_have_results
  global use_windowed_mode
  
  clean_results 
  clean_canvas

  if {[write_input_file] != 0} {
    tk_messageBox -icon error -type ok -message "Solution aborted."
    return -1
  }

  set cmdline $monte_exe

  if {$i_file_name == ""} {
     tk_messageBox -icon error -type ok -message "No data to run!"
     return -1
  }

  # command:
  append cmdline " -v "

  # input file:
  append cmdline " -i " $i_file_name
  append cmdline " -s " $sim_number

  # numbers:
  if {$sim_verbose > 1} { append cmdline " -vn " $sim_verbose }
  if {$sim_test > 1} {
    if {$sim_test > $sim_number} {set sim_test $sim_number}
    append cmdline " -fon " $sim_test 
  }
  if {$sim_wall > 1} { append cmdline " -wall " $sim_wall }

  # files:
  if {$o_stat_name != ""} { append cmdline " -fs " $o_stat_name }
  if {$sim_savesim == 1} {
    if {$o_sim_name != ""} { append cmdline " -nofrh -fr " $o_sim_name }
  }
  if {$o_h_f != ""} { append cmdline " -foh " $o_h_f }

  # library:
  if {$i_library != ""} { append cmdline " -ld " $i_library }
  if {$i_libarg != ""} { append cmdline " -lda " $i_libarg }
  #puts $cmdline

  if {$real_run == 0} {
    tk_messageBox -type ok -title "Command to run" -message $cmdline
    puts "# Command line to run:"
    puts $cmdline
    return 0
  }

  # hide window:
  wm iconify .
  if {$use_windowed_mode == 1} {
    wm iconify .iv
    wm iconify .ov
  }

  if {$real_run == 1} {
    #exec echo "$cmdline ; sleep 2" > $runmonte
    exec xterm -title "Monte solver: $i_file_name" -e /bin/sh "$cmdline ; sleep 3"
  } 

  wm deiconify .
  if {$use_windowed_mode == 1} {
    wm deiconify .iv
    wm deiconify .ov
  }

  tk_messageBox -type ok -title "Solution is done" -message "Solution is done!"
  set we_have_results 1 

	if  {[read_results] != 0} {
    tk_messageBox -icon error -type ok -message "I/O Error on statistics file - no results will be displayed!"
		set we_have_results 0 
}

  return 0
}

# "help About" dialog:
proc helpAbout {} {
  tk_messageBox -type ok -title "Ahout" -message "Monte: Reliability Tool\n(C) 2006: \nJiri Brozovsky, \nPetr Konecny, \nJakub Valihrach"
}

proc plot_result_hist {} {
  global  we_have_results ovname o_h_f dis2tk

  if {$we_have_results == 1} {
    if {$o_h_f != ""} {
      set name $o_h_f
      append name $ovname
      append name ".dis"
      exec $dis2tk $name
      append name ".tk"
      source  $name
      gnuplot .sv.c
    } else {
      clean_canvas
    }
  }
}

proc clean_canvas {} {
  .sv.c delete all
}

proc get_false_results {} {
  global we_have_results
  global i_file_name
  global o_h_f

  if {$i_file_name != ""} {
    if {$o_h_f != ""} {
      set we_have_results 1

			read_results
    }
  }
}

proc hide_results {} {
	global rlabel1 rlabel2

	set rlabel1 ""
	set rlabel2 ""
}

proc clean_results {} {
	global we_have_results
	global rs_len  rs_sims
	global rs_min  rs_max  rs_aver rs_disp rs_stdev rs_varc 
	global rlabel1 rlabel2

	unset rs_min   
	unset rs_max   
	unset rs_aver  
	unset rs_disp  
	unset rs_stdev 
	unset rs_varc  

	set rs_len  0
	set rs_sims 0
	set rs_min   [list]
	set rs_max   [list]
	set rs_aver  [list]
	set rs_disp  [list]
	set rs_stdev [list]
	set rs_varc  [list]

	set rlabel1 ""
	set rlabel2 ""

	set we_have_results 0
}

proc read_results {} {
	global we_have_results
	global o_stat_name
	global rs_len  rs_sims
	global rs_min  rs_max  rs_aver rs_disp rs_stdev rs_varc 

	set line [list]

	clean_results

	if {$o_stat_name == ""} {
		return -1
	}

	# open file
	if [ catch {set fr [open $o_stat_name "r"]} result] {
    #puts $result  
    tk_messageBox -icon error -type ok -message "Can not open statistics file!"
    return -1 
  }

	# read data 
  gets $fr line

	set rs_len [lindex $line 0]
	set rs_sims [lindex $line 1]

	if {$rs_len <= 0} { return -1 }
	if {$rs_sims <= 0} { return -1 }

  for {set i 0} {$i< $rs_len} {incr i} {
  	gets $fr line

    set min [lindex $line 0]
    set max [lindex $line 1]
    set aver [lindex $line 2]
    set dispers [lindex $line 3]
    set stddev [lindex $line 4]
    set varc [lindex $line 5]

		lappend rs_min $min
		lappend rs_max $max
		lappend rs_aver $aver
		lappend rs_disp $dispers
		lappend rs_stdev $stddev
		lappend rs_varc $varc
	}
     
  # close file:
  if [ catch [close $fr] result] { 
    tk_messageBox -icon error -type ok -message "Can not close statistics file!"
    #puts $result 
    return -1 
  }

	set we_have_results 1
	return 0
}

proc show_result { i } {
	global rs_len  rs_sims we_have_results
	global rs_min  rs_max  rs_aver rs_disp rs_stdev rs_varc 
	global rlabel1 rlabel2
	
	if {$we_have_results != 1} { return }	
	if {$rs_len <= 0} { return }
	if {$i >= $rs_len} { return }

	# top line
	set rlabel1 ""
	append rlabel1 "sims: "
	append rlabel1 $rs_sims
	append rlabel1 ", aver: "
	append rlabel1 [format "%e  " [lindex $rs_aver $i]]
	append rlabel1 "< "
	append rlabel1 [format "%e" [lindex $rs_min $i]]
	append rlabel1 " ; "
	append rlabel1 [format "%e" [lindex $rs_max $i]]
	append rlabel1 " >"

	# bottom line
	set rlabel2 ""
	append rlabel2 "disp: "
	append rlabel2 [format "%e " [lindex $rs_disp $i]]
	append rlabel2 ", std.d.: "
	append rlabel2 [format "%e " [lindex $rs_stdev $i]]
	append rlabel2 ", var.c.: "
	append rlabel2 [format "%e " [lindex $rs_varc $i]]
}


#### ************************************************************* ####
#### ************************************************************* ####
#### ************************************************************* ####


#### Input variables window ********************************

if {$use_windowed_mode == 1} {
  toplevel .iv ;#-background red
  wm title .iv "Input Variables"
} else {
  frame .iv -borderwidth 4 -relief sunken
  pack .iv -side left -fill "both"

  label .iv.title -text "Input Variables" -justify "center" ;#-font bold
  grid .iv.title -row 0 -column 0  -columnspan 3
}

scrollbar .iv.h -orient horizontal -command ".iv.ivlist xview"
scrollbar .iv.v -command ".iv.ivlist yview"

listbox .iv.ivlist -yscroll ".iv.v set" -xscroll ".iv.h set"

# listbox with scrollbars
grid .iv.ivlist -row 1 -column 0 -columnspan 2 -sticky "news"
grid .iv.v -row 1 -column 2 -sticky "ns"
grid .iv.h -row 2 -column 0 -columnspan 3 -sticky "we"

grid columnconfigure .iv 1 -weight 1
grid rowconfigure .iv 1 -weight 1


# name:
label .iv.namlab -text "Variable name: " 
entry .iv.name -textvariable varname

grid .iv.namlab -row 3 -column 0 -sticky w
grid .iv.name -row 3 -column 1 -columnspan 2 -sticky w

# constant:
label .iv.conlab -text "Multiplier: " 
entry .iv.conent -textvariable cmult

grid .iv.conlab -row 4 -column 0 -sticky w
grid .iv.conent -row 4 -column 1 -columnspan 2 -sticky w

# empty label (space):
label .iv.empty2 -text ""
grid .iv.empty2 -row 5 -column 0 -columnspan 3

# type selection:
label .iv.datype -text "Data type: "
radiobutton .iv.constant -text "constant" -variable ivtype -value "const" -justify left
radiobutton .iv.his -text "histogram" -variable ivtype -value "his" -justify left
radiobutton .iv.copy -text "copy" -variable ivtype -value "hcopy" -justify left

grid .iv.datype -row 6 -column 0 -sticky w
grid .iv.constant -row 6 -column 1 -columnspan 2 -sticky w
grid .iv.his -row 7 -column 1 -columnspan 2 -sticky w
grid .iv.copy -row 8 -column 1 -columnspan 2 -sticky w

proc find_dis_name {} {
  global his_dir hisname ivtype

  set name ""
  set ivtype "his"
  
  set types { 
    {{Histogram files}       {.dis}        TEXT} 
    {{Histogram files}       {.DIS}        TEXT} 
  }
  
  set name [tk_getOpenFile -defaultextension  ".dis" -filetypes $types -initialdir $his_dir ]

  if {$name == ""} {
    return -1
  }

  #puts $name
  #puts [file dirname $name]

  #set hisname [concat [file rootname $name] "." [file extension $name]]
  if {[file dirname $name] != ""} {
    set from [expr [string length [file dirname $name]]  + 1]
    set to [expr [string length $name] - 1]
    set hisname [string range $name $from $to]
  } else {
    set hisname $name
  }
  ##set hisname [file dirname $name]
  #puts [file dirname $his_dir]
  #puts $hisname
}


# empty label (space):
label .iv.empty3 -text ""
grid .iv.empty3 -row 9 -column 0 -columnspan 3


# histogram file:
label .iv.hisfl -text "Histogram: "
label .iv.hname -textvariable hisname
#button .iv.hsel -text "Select File" -command find_dis_name
grid .iv.hisfl -row 10 -column 0 -sticky w
grid .iv.hname -row 10 -column 1 -sticky w
#grid .iv.hsel -row 10 -column 1 -sticky w

frame .iv.hobox
grid .iv.hobox -row 11 -column 1 -sticky w
button .iv.hobox.sel -text "Select File" -command find_dis_name
button .iv.hobox.show -text "View File" -command show_i_dis
pack .iv.hobox.sel -side left
pack .iv.hobox.show -side left

# empty label (space):
label .iv.empty1 -text ""
grid .iv.empty1 -row 12 -column 0 -columnspan 3

# New/Change/Delete buttons:
frame .iv.but 
grid .iv.but -row 13 -column 0 -columnspan 3

button .iv.but.new   -text "New" -command "new_i"
button .iv.but.apply -text "Change" -command "edit_i"
button .iv.but.del   -text "Delete" -command "delete_i"

pack .iv.but.new   -side left
pack .iv.but.apply -side left
pack .iv.but.del   -side left


###

#### Output variables window ********************************

if {$use_windowed_mode == 1} {
  toplevel .ov ;#-background red
  wm title .ov "Output Variables"
} else {
  frame .ov -borderwidth 4 -relief sunken
  pack .ov -side left -fill "both"

  label .ov.title -text "Output Variables" -justify "center" ;#-font bold
  grid .ov.title -row 0 -column 0  -columnspan 3

}

scrollbar .ov.h -orient horizontal -command ".ov.ovlist xview"
scrollbar .ov.v -command ".ov.ovlist yview"

listbox .ov.ovlist -yscroll ".ov.v set" -xscroll ".ov.h set"


# listbox with scrollbars
grid .ov.ovlist -row 1 -column 0 -columnspan 2 -sticky "news"
grid .ov.v -row 1 -column 2 -sticky "ns"
grid .ov.h -row 2 -column 0 -columnspan 3 -sticky "we"

grid columnconfigure .ov 1 -weight 1
grid rowconfigure .ov 1 -weight 1


# name:
label .ov.namlab -text "Variable name: "
entry .ov.name -textvariable ovname

grid .ov.namlab -row 3 -column 0 -sticky w
grid .ov.name -row 3 -column 1 -columnspan 2 -sticky w

# empty label (space):
label .ov.empty2 -text ""
grid .ov.empty2 -row 5 -column 0 -columnspan 3

# type selection:
label .ov.datype -text "Histogram type: "
radiobutton .ov.htnone -text "unused" -variable ovtype -value "0" -justify left
radiobutton .ov.htauto -text "automatic" -variable ovtype -value "1" -justify left
radiobutton .ov.htautodyn -text "automatic, dynamic" -variable ovtype -value "2" -justify left
radiobutton .ov.htnum -text "custom size" -variable ovtype -value "3" -justify left
radiobutton .ov.htnumdyn -text "custom size, dynamic" -variable ovtype -value "4" -justify left
radiobutton .ov.htlim -text "fixed" -variable ovtype -value "5" -justify left
radiobutton .ov.htlimdyn -text "fixed, dynamic" -variable ovtype -value "6" -justify left

grid .ov.datype -row 6 -column 0 -sticky w

grid .ov.htnone -row 6 -column 1 -columnspan 2 -sticky w
grid .ov.htauto -row 7 -column 1 -columnspan 2 -sticky w
grid .ov.htautodyn -row 8 -column 1 -columnspan 2 -sticky w
grid .ov.htnum -row 9 -column 1 -columnspan 2 -sticky w
grid .ov.htnumdyn -row 10 -column 1 -columnspan 2 -sticky w
grid .ov.htlim -row 11 -column 1 -columnspan 2 -sticky w
grid .ov.htlimdyn -row 12 -column 1 -columnspan 2 -sticky w

# empty label (space):
label .ov.empty3 -text ""
grid .ov.empty3 -row 13 -column 0 -columnspan 3


# intervals
label .ov.intvlab -text "Intervals: "
entry .ov.intv -textvariable ovnum

grid .ov.intvlab -row 14 -column 0 -sticky w
grid .ov.intv -row 14 -column 1 -columnspan 2 -sticky w

# from
label .ov.fromlab -text "From: "
entry .ov.from -textvariable ovfrom

grid .ov.fromlab -row 15 -column 0 -sticky w
grid .ov.from -row 15 -column 1 -columnspan 2 -sticky w

# to
label .ov.tolab -text "To: "
entry .ov.to -textvariable ovto

grid .ov.tolab -row 16 -column 0 -sticky w
grid .ov.to -row 16 -column 1 -columnspan 2 -sticky w


# empty label (space):
label .ov.empty4 -text ""
grid .ov.empty4 -row 17 -column 0 -columnspan 3


# New/Change/Delete buttons:
frame .ov.but 
grid .ov.but -row 18 -column 0 -columnspan 3

button .ov.but.new   -text "New" -command "new_o"
button .ov.but.apply -text "Change" -command "edit_o"
button .ov.but.del   -text "Delete" -command "delete_o"

pack .ov.but.new   -side left
pack .ov.but.apply -side left
pack .ov.but.del   -side left

#### Canvas and solution window *****************************

if {$use_windowed_mode == 666} {
  toplevel .sv ;#-background red
  wm title .sv "Solution Control"
} else {
  frame .sv -borderwidth 4 -relief sunken
  pack .sv -side left -fill "both" -expand 1
}

# line with results
frame  .sv.rlab
grid .sv.rlab -row 0 -column 0 -columnspan 11  -sticky "news"

label .sv.rlab.l1  -background white -foreground black -textvariable rlabel1
label .sv.rlab.l2  -background white -foreground black -textvariable rlabel2

pack .sv.rlab.l1 -side top -fill both
pack .sv.rlab.l2 -side top -fill both


# canvas:

canvas .sv.c -background white

grid .sv.c -row 1 -column 0 -columnspan 11  -sticky "news"

grid columnconfigure .sv 1 -weight 1
grid rowconfigure .sv 1 -weight 1

# library binary:
label .sv.liblab -text "Library: "
label .sv.lib -textvariable i_library ;#-width 10
button .sv.libbut -text "Set"  -command "setLib"

grid .sv.liblab -row 2 -column 0 -sticky w
grid .sv.lib -row 2 -column 1 -columnspan 6 -sticky w
grid .sv.libbut -row 2 -column 7 -sticky w


# library argument:
label .sv.libarglab -text "Arg.: "
entry .sv.libarg -textvariable i_libarg ;#-width 40
button .sv.libargbut -text "Set"  -command "setLibArg"

grid .sv.libarglab -row 3 -column 0 -sticky w
grid .sv.libarg -row 3 -column 1 -columnspan 6 -sticky "ew"
grid .sv.libargbut -row 3 -column 7 -sticky w


# simulations:
label .sv.simlab -text "Simulations: "
entry .sv.sim -textvariable sim_number -width 10

grid .sv.simlab -row 4 -column 0 -sticky w
grid .sv.sim -row 4 -column 1 -sticky w


# verbose number of simulations:
label .sv.verlab -text "Verbose sim.: "
entry .sv.ver -textvariable sim_verbose -width 6

grid .sv.verlab -row 4 -column 3 -sticky w
grid .sv.ver -row 4 -column 4 -sticky w


# simulations to adjust histograms:
label .sv.testlab -text "H. test sim.: "
entry .sv.test -textvariable sim_test -width 6

grid .sv.testlab -row 4 -column 6 -sticky w
grid .sv.test -row 4 -column 7 -sticky w

# max. run time:
label .sv.walllab -text "Wall \[days\]: "
entry .sv.wall -textvariable sim_wall -width 4

grid .sv.walllab -row 5 -column 0 -sticky w
grid .sv.wall -row 5 -column 1 -sticky w

# max. run time:
checkbutton .sv.savsim -text "Save log"\
-variable sim_savesim -onvalue 1 -offvalue 0 

grid .sv.savsim -row 5 -column 3 -columnspan 2 -sticky w

# "Run" button

button .sv.runsim -text "Run simulation" -command "simRun 1" 

grid .sv.runsim -row 5 -column 6 -columnspan 4 -sticky "news"


### ########################################################
### Main Window ********************************************

# Menu system  
menu .mbar -tearoff 0
    . configure -menu .mbar

  .mbar add cascade -label "File" -menu .mbar.file -underline 0
    menu .mbar.file -title "File" -tearoff 1 
    .mbar.file add command -label "New"  -command "fileNew" -accelerator "Ctrl+N" -underline 0
    .mbar.file add separator
    .mbar.file add command -label "Open..."  -command "fileOpen" -accelerator "Ctrl+O" -underline 0
    .mbar.file add separator
    .mbar.file add command -label "Save"  -command "write_input_file" -accelerator "Ctrl+S" -underline 0
    .mbar.file add command -label "Save As..." -command "fileSaveAs"  -accelerator "Ctrl+Shift+S"
    .mbar.file add separator
    .mbar.file add command -label "Quit" -command exit -accelerator "Ctrl+Q" -underline 0

 .mbar add cascade -label "Simulate" -menu .mbar.sim -underline 0
    menu .mbar.sim -title "Simulate" -tearoff 1
    .mbar.sim add command -label "Run simulation(s)"  -command "simRun 1" -accelerator "Ctrl+R" -underline 0

 .mbar add cascade -label "Tools" -menu .mbar.tools -underline 0
    menu .mbar.tools -title "Tools" -tearoff 1
    .mbar.tools add command -label "Clear canvas"  -command "clean_canvas ; hide_results" -accelerator "Ctrl+E" -underline 2
    .mbar.tools add separator
    .mbar.tools add command -label "Show command line"  -command "simRun 0" -accelerator "Ctrl+V" -underline 2
    .mbar.tools add separator
    .mbar.tools add command -label "Enable old result histograms"  -command get_false_results
    .mbar.tools add command -label "Disable result histograms"  -command "set we_have_results 0 ; clean_canvas"



 .mbar add cascade -label "Help" -menu .mbar.help -underline 0
    menu .mbar.help -title "Help" -tearoff 0
    .mbar.help add command -label "About"  -command "helpAbout"

# bindings:
bind .iv.ivlist <ButtonRelease-1> { refresh_i_list }
bind .ov.ovlist <ButtonRelease-1> { refresh_o_list }

bind . <Control-n>  {fileNew}
bind . <Control-o>  {fileOpen}
bind . <Control-s>  {write_input_file}
bind . <Control-S>  {fileSaveAs}
bind . <Control-r>  {simRun 1}
bind . <Control-e>  {clean_canvas ; hide_results }
bind . <Control-q>  {exit}


# Initialization: ######################################################

#tk_bisque
#tk_setPalette background lightsteelblue foreground black
#tk_setPalette background lightgray foreground black
#tk_setPalette background snow foreground black
tk_setPalette background gray foreground black

wm title . "Monte Reliability Research Tool"
