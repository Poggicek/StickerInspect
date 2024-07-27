set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_DEBUG_POSTFIX d)

add_library(imgui STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx9.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp
)

target_include_directories(imgui PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/imgui
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(imgui d3d9)