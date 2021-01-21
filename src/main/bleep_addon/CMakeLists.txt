project(bleep_addon)

SET(BLEEP_ADDON_SRC
    bitcoin_mine_support/coinflip_validation.cpp
    bleep_addon.cpp
    bitcoin_log_map/log_map.cpp
    bitcoin_storage_share/storage_share.cpp
)
SET(BLEEP_ADDON_STORAGE_SHARE_SRC
    storage_share/src/core/util/utils.cpp
    storage_share/src/core/datatypes/datatype.cpp
    storage_share/src/core/data_unit/data_segment.cpp
    storage_share/src/core/share/sharing_tracker.cpp
    storage_share/src/core/file/file.cpp
    storage_share/src/core/file/file_descriptor.cpp
    storage_share/src/core/file/file_tracker.cpp
    storage_share/src/core/file_control.cpp
)

add_library(bleep_addon SHARED ${BLEEP_ADDON_SRC} ${BLEEP_ADDON_STORAGE_SHARE_SRC})
install(TARGETS bleep_addon DESTINATION lib)
