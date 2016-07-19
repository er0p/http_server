cmake_minimum_required (VERSION 2.8)
project (http_server)

#############################
## Compiler flags.
#############################
#list( APPEND CMAKE_CXX_FLAGS "-pthread -std=c++0x -march=native -O2 -g -Wall ${CMAKE_CXX_FLAGS} ")
list( APPEND CMAKE_CXX_FLAGS "-pthread -std=c++0x -march=native -ggdb3 -Wall ${CMAKE_CXX_FLAGS} ")

##########
# Src
##########

#SET (net_utils_LIB net_utils)
#SET (net_utils_SRC net_utils.cpp)

#SET (HTTP_SRC http_server.cpp net_utils.cpp)
#SET (HTTP_SRC http_server1.cpp net_utils.cpp)
SET (HTTP_SRC http_server2.cpp net_utils.cpp)
SET (HTTP_EXECUTABLE final)
message ("src: ${HTTP_SRC}")


#ADD_LIBRARY(${net_utils_LIB} SHARED ${net_utils_SRC})

ADD_EXECUTABLE(${HTTP_EXECUTABLE} ${HTTP_SRC})

#TARGET_LINK_LIBRARIES(${HTTP_EXECUTABLE} ${net_utils_LIB})
#TARGET_LINK_LIBRARIES(${HTTP_EXECUTABLE} uuid)
#TARGET_LINK_LIBRARIES(${HTTP_EXECUTABLE} ev)
