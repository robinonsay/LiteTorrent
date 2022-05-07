# Make a library
add_library(lite_torrent argparse.cpp crypto.cpp errors.cpp ltdefs.cpp ltpeer.cpp mrsw_mutex.cpp tcp.cpp)
# Add includes
target_include_directories(lite_torrent PUBLIC ${CMAKE_SOURCE_DIR}/include ${OPENSSL_DIR}/include)
# Add dependencies
target_link_libraries(lite_torrent ${OPENSSL_DIR}/lib64/libcrypto.so ${OPENSSL_DIR}/lib64/libssl.so Threads::Threads)
