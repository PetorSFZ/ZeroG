set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(INCLUDE_FILES
	${INCLUDE_DIR}/d3dx12.h
)
add_custom_target(D3DX12 SOURCES ${INCLUDE_FILES})

set(D3DX12_FOUND true PARENT_SCOPE)
set(D3DX12_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
