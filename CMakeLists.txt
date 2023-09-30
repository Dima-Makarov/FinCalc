cmake_minimum_required(VERSION 3.17)
project(FinCalc)

set(CMAKE_CXX_STANDARD 17)

set(IMGUI_SRC 	lib/imgui.cpp
		lib/imgui_demo.cpp
		lib/imgui_draw.cpp
		lib/imgui_impl_win32.cpp
		lib/imgui_tables.cpp
		lib/imgui_widgets.cpp
		lib/imgui_impl_opengl3.cpp
)
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp src/fin_calc.cpp ${IMGUI_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ./src)
target_include_directories(${PROJECT_NAME} PUBLIC ./lib)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
configure_file(./font/arial.ttf ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)