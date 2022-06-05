set(PARALLEL_RDP_STANDALONE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/parallel-rdp)

set(PARALLEL_RDP_SRC_FILES
    ${PARALLEL_RDP_STANDALONE_DIR}/parallel-rdp/command_ring.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/parallel-rdp/rdp_device.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/parallel-rdp/rdp_dump_write.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/parallel-rdp/rdp_renderer.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/parallel-rdp/video_interface.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/buffer.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/buffer_pool.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/command_buffer.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/command_pool.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/context.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/cookie.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/descriptor_set.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/device.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/event_manager.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/fence.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/fence_manager.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/image.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/memory_allocator.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/pipeline_event.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/query_pool.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/render_pass.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/sampler.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/semaphore.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/semaphore_manager.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/shader.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan/texture_format.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/util/logging.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/util/thread_id.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/util/aligned_alloc.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/util/timer.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/util/timeline_trace_file.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/util/thread_name.cpp
    ${PARALLEL_RDP_STANDALONE_DIR}/volk/volk.c
)

set(PARALLEL_RDP_INCLUDE_DIRS
    ${PARALLEL_RDP_STANDALONE_DIR}/parallel-rdp
    ${PARALLEL_RDP_STANDALONE_DIR}/volk
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan
    ${PARALLEL_RDP_STANDALONE_DIR}/vulkan-headers/include
    ${PARALLEL_RDP_STANDALONE_DIR}/util
)

set(PARALLEL_RDP_DEFS NOMINMAX GRANITE_VULKAN_MT)

if (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
    set(PARALLEL_RDP_LIBS dl)
elseif (MSVC)
    set(PARALLEL_RDP_LIBS winmm)
    list(APPEND PARALLEL_RDP_DEFS VK_USE_PLATFORM_WIN32_KHR)
endif()

add_library(parallel-rdp-standalone STATIC ${PARALLEL_RDP_SRC_FILES})
target_link_libraries(parallel-rdp-standalone PUBLIC ${PARALLEL_RDP_LIBS})
target_compile_options(parallel-rdp-standalone PRIVATE ${PJ64_PARALLEL_RDP_CXX_FLAGS})
target_include_directories(parallel-rdp-standalone PUBLIC ${PARALLEL_RDP_INCLUDE_DIRS})
target_compile_definitions(parallel-rdp-standalone PUBLIC ${PARALLEL_RDP_DEFS})
