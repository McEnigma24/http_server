cmake_minimum_required(VERSION 3.18)
project(MyProject LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# set(CMAKE_CXX_COMPILER g++)
# set(CMAKE_CXX_COMPILER clang++)

include_directories(_inc)
aux_source_directory(_src SOURCES)


# MACRO FOR LOGS # - dont show the whole realpath to file

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions(-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})


# COMPILER FLAGS #

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")                                                             # to see everything
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wfatal-errors -Werror=uninitialized -Werror=init-self -Werror=reorder")    # for normal coding
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -w")                                                         # for release

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")





# SUBMODULES #

# 1 - httpparser #
include(FetchContent)
FetchContent_Declare(
    httpparser
    GIT_REPOSITORY https://github.com/nekipelov/httpparser.git
    GIT_TAG        master # lub inny tag/commit
)
FetchContent_Populate(httpparser)

# set(ALL_LIBRARIES ${ALL_LIBRARIES} httpparser)
include_directories(${httpparser_SOURCE_DIR}/src)



# 2 - json #
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        master # lub inny tag/commit
)
FetchContent_Populate(json)

# set(ALL_LIBRARIES ${ALL_LIBRARIES} json)
include_directories(${json_SOURCE_DIR}/include)



# 3 - gpiod #
set(ALL_LIBRARIES ${ALL_LIBRARIES} gpiod)



# 4 - ssd1306 #
include(FetchContent)
FetchContent_Declare(
    ssd1306
    GIT_REPOSITORY https://github.com/armlabs/ssd1306_linux.git
    GIT_TAG        master # lub inny tag/commit
)
FetchContent_MakeAvailable(ssd1306) # tutaj nie dzial, a normanie zbudowalby projekt, ale tutaj nie zrobi bo nie ma pliku CMakeLists.txt

include_directories(${ssd1306_SOURCE_DIR})

# Dodaj pliki źródłowe ręcznie
file(GLOB SSD1306_SOURCES
    ${ssd1306_SOURCE_DIR}/*.c
)

add_library(ssd1306 STATIC ${SSD1306_SOURCES})
target_include_directories(ssd1306 PUBLIC ${ssd1306_SOURCE_DIR})

set(ALL_LIBRARIES ${ALL_LIBRARIES} ssd1306) # dodajemy do glownego EXE


set(CONST_TARGET_NAME "HTTP_SERVER")
    message(WARNING "Building exe")
    set(TARGET_NAME "${CONST_TARGET_NAME}.exe")
    add_executable(${TARGET_NAME} ${SOURCES})
    target_compile_definitions(${TARGET_NAME} PRIVATE LINUX BUILD_EXECUTABLE)
    # set_target_properties(${TARGET_NAME} PROPERTIES ${COMPILE_FLAGS} ${LINK_FLAGS})
    target_link_libraries(${TARGET_NAME} PUBLIC ${ALL_LIBRARIES})
