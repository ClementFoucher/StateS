set(dtl_header_files
    "dtl/dtl/variables.hpp"
    "dtl/dtl/functors.hpp"
    "dtl/dtl/Sequence.hpp"
    "dtl/dtl/Lcs.hpp"
    "dtl/dtl/Ses.hpp"
    "dtl/dtl/Diff.hpp"
    "dtl/dtl/Diff3.hpp"
    "dtl/dtl/dtl.hpp"
)

set(dtl_include_directories
    "dtl/dtl"
)

qt_add_library(dtl INTERFACE)

target_sources(dtl INTERFACE ${dtl_header_files})

target_include_directories(dtl INTERFACE ${dtl_include_directories})
