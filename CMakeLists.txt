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
set(BIG_INT_SRC
		big_int/big_int.cc
		big_int/big_int_base_converter.cc
		big_int/big_int_ctors_dtor.cc
		big_int/big_int_priv_defs.cc
)
find_package(OpenGL REQUIRED)
find_package(Threads)

add_executable(${PROJECT_NAME} src/main.cpp src/fin_calc.cpp ${IMGUI_SRC} ${BIG_INT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ./src ./lib ./big_int)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})