file(REMOVE_RECURSE
  "../lib/libgoa-json.a"
  "../lib/libgoa-json.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/goa-json.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
