function(remove_flag variable flag_to_remove)
  # Get the current value of the variable
  set(current_value "${${variable}}")
  # Delete flag with leading or trailing space
  string(REPLACE " ${flag_to_remove}" "" current_value "${current_value}")
  string(REPLACE "${flag_to_remove} " "" current_value "${current_value}")
  set(${variable} "${current_value}" PARENT_SCOPE)
endfunction()