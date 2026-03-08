set(MAILIO_SRCS
    base64.cpp
    binary.cpp
    bit7.cpp
    bit8.cpp
    codec.cpp
    dialog.cpp
    mailboxes.cpp
    message.cpp
    mime.cpp
    percent.cpp
    q_codec.cpp
    quoted_printable.cpp
    smtp.cpp
)

add_library(mailio STATIC ${MAILIO_SRCS})

# mailio needs ASIO_STANDALONE and Boost (for algorithm/string)
target_compile_definitions(mailio PUBLIC ASIO_STANDALONE)

target_include_directories(mailio PUBLIC 
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(mailio PUBLIC 
    OpenSSL::SSL 
    OpenSSL::Crypto
    Threads::Threads
)

# Installation (optional, based on old one)
install(TARGETS mailio
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
