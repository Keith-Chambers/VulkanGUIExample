cmake_minimum_required(VERSION 3.8)

project(vulkanGui-v0.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SRC_PATH ${CMAKE_SOURCE_DIR}/)

add_executable(
    ${PROJECT_NAME}
    initvulkan.cpp
    mainvulkan.cpp
    vulkanhelper.cpp
    text.cpp
    config.cpp
)

# target_compile_options(${PROJECT_NAME} PRIVATE -pg)

link_directories(${CMAKE_SOURCE_DIR}/lib)
                                
target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_PATH}/include)

target_link_libraries(${PROJECT_NAME} "-lglfw")
target_link_libraries(${PROJECT_NAME} "-lvulkan")
target_link_libraries(${PROJECT_NAME} "-lfreetype")






