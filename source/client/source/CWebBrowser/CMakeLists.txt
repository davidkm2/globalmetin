cmake_minimum_required(VERSION 3.2)
project(CWebBrowser)

file(GLOB CWebBrowser_HEADERS
    "*.h"
    "*.hpp"
)
file(GLOB CWebBrowser_SOURCES
    "*.c"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC ${CWebBrowser_HEADERS} ${CWebBrowser_SOURCES})
