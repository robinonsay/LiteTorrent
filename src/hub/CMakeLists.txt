# Make a peer executable
add_executable(hub hub_main.cpp hub.cpp)
# Add dependencies
target_link_libraries(hub LINK_PUBLIC lite_torrent)
