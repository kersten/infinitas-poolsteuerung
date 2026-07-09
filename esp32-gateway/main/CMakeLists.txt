set(POOL_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}/../../lib/pool-common/src")

idf_component_register(
    SRCS
        "main.cpp"
        "${POOL_COMMON_DIR}/ChannelState.cpp"
        "${POOL_COMMON_DIR}/Protocol.cpp"
    INCLUDE_DIRS
        "."
        "${POOL_COMMON_DIR}"
    PRIV_REQUIRES
        driver
        esp_matter
        nvs_flash)

target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
