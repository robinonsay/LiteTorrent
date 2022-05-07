# Make a peer executable
add_library(mutex mrsw_mutex.cpp)
# Add dependencies
target_link_libraries(mutex LINK_PUBLIC lite_torrent)
