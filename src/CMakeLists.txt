# Make a library
add_library(lite_torrent argparse.cpp crypto.cpp errors.cpp ltdefs.cpp tcp.cpp)
# Add includes
target_include_directories(lite_torrent PUBLIC ${CMAKE_SOURCE_DIR}/include ${OPENSSL_DIR}/include)
# Add dependencies
target_link_libraries(lite_torrent ${OPENSSL_DIR}/lib64/libcrypto.so ${OPENSSL_DIR}/lib64/libssl.so Threads::Threads)

add_subdirectory(hub)
add_subdirectory(mutex)
add_subdirectory(peer)

# Import OpenSSL library
# add_library(openssl_crypto STATIC IMPORTED)
# set_property(TARGET openssl_crypto PROPERTY IMPORTED_LOCATION "${OPENSSL_DIR}/lib64/libcrypto.a")
# add_library(openssl_ssl STATIC IMPORTED)
# set_property(TARGET openssl_ssl PROPERTY IMPORTED_LOCATION "${OPENSSL_DIR}/lib64/libssl.a")
# # Make a library
# add_library(lite_torrent argparse.cpp crypto.cpp errors.cpp ltdefs.cpp tcp.cpp)
# # Add includes
# target_include_directories(lite_torrent PUBLIC ${CMAKE_SOURCE_DIR}/include ${OPENSSL_DIR}/include)
# # Add dependencies
# target_link_libraries(lite_torrent  PUBLIC openssl_crypto openssl_ssl)
