SET(BLEEP_ADDON_SRC
    bleep_addon/bleep_addon.cpp
    bleep_addon/bitcoin_log_map/log_map.cpp
    bleep_addon/bitcoin_storage_share/storage_share.cpp
)
add_library(bleep_addon SHARED ${BLEEP_ADDON_SRC})
install(TARGETS bleep_addon DESTINATION lib)
