# Include directories
set(include_dirs include include/port include/port/coap)

# Source code files
set(srcs
    "src/address.c"
    "src/async.c"
    "src/block.c"
    "src/coap_event.c"
    "src/coap_hashkey.c"
    "src/coap_session.c"
    "src/coap_time.c"
    "src/coap_debug.c"
    "src/encode.c"
    "src/mem.c"
    "src/net.c"
    "src/option.c"
    "src/pdu.c"
    "src/resource.c"
    "src/str.c"
    "src/subscribe.c"
    "src/uri.c"
    "src/coap_notls.c"
    "src/coap_io.c"
)

# Register component in the IDF
idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "${include_dirs}"
    REQUIRES lwip
)

# Originally, this file triggers ggc's 'format-truncation' warning
set_source_files_properties(src/coap_debug.c PROPERTIES COMPILE_FLAGS -Wno-format-truncation)

# For some reason it may be required by the build as long as the macro WITH_POSIX is used
target_compile_definitions(${COMPONENT_LIB} PUBLIC WITH_POSIX)
