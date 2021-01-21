set(srcs
        util/utils.cpp
        datatypes/datatype.cpp
        data_unit/data_segment.cpp
        share/sharing_tracker.cpp
        file/file.cpp
        file/file_descriptor.cpp
        file/file_tracker.cpp
        file_control.cpp
        )

add_library(storage_share SHARED ${srcs})