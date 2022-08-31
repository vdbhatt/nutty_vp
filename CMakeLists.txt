cmake_minimum_required(VERSION 3.10)

project(nutty_vp)

set(SRC
    src/bus_interconnect.cpp
    src/rv32i_iss.cpp
    src/rv32i.cpp
    src/simple_memory_interface.cpp
    src/soc_memory.cpp
    src/top.cpp
    src/dot_product.cpp
    )

include_directories(/opt/systemc-2.3.3/include)
link_directories(/opt/systemc-2.3.3/lib-linux64)

add_executable(main ${SRC} src/main.cpp)
include_directories(include)
include_directories(/workspaces/nutty_vp/libs/spdlog/include)

target_link_libraries(main PRIVATE systemc)