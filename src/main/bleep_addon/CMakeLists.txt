project(bleep_addon)

SET(BLEEP_ADDON_SRC
        bleep_addon.cpp
        )
SET(BLEEP_ADDON_MEMORY_SHARE_SRC
        memshare/memory_sharing.cpp
        )

add_library(bleep_addon SHARED ${BLEEP_ADDON_SRC} ${BLEEP_ADDON_MEMORY_SHARE_SRC})
install(TARGETS bleep_addon DESTINATION lib)
