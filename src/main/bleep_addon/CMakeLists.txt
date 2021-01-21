SET(BLEEP_ADDON_SRC
    bitcoin_mine_support/coinflip_validation.cpp
    bleep_addon.cpp
    bitcoin_log_map/log_map.cpp
    bitcoin_storage_share/storage_share.cpp
)
add_library(bleep_addon SHARED ${BLEEP_ADDON_SRC})
install(TARGETS bleep_addon DESTINATION lib)
