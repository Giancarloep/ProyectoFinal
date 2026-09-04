cmake_minimum_required(VERSION 4.3)
project(ProjectoFinal C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(nucleo STATIC
        src/timer.c
        src/GeneradorRutinas.cpp
        src/TrackingPR.cpp
        src/RepositorioMemoria.cpp
        src/TablaHipertrofia.cpp
        src/TiersSaiyayin.cpp
)
target_include_directories(nucleo PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_compile_definitions(nucleo PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
if(WIN32)
    target_link_libraries(nucleo PUBLIC ws2_32 crypt32)
endif()

add_library(api STATIC
        api/Servicio.cpp
        api/ApiRest.cpp
        api/Asistente.cpp
        api/CerebroIA.cpp
)
target_include_directories(api PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${CMAKE_CURRENT_SOURCE_DIR}/api
)
target_link_libraries(api PUBLIC nucleo)
if(WIN32)
        target_link_libraries(api PUBLIC ws2_32)
endif()
target_compile_definitions(api PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)

add_executable(ProjectoFinal main.cpp)
target_link_libraries(ProjectoFinal PRIVATE api)

if(WIN32)
        get_filename_component(MINGW_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(dll libstdc++-6.dll libgcc_s_seh-1.dll libwinpthread-1.dll)
                if(EXISTS "${MINGW_BIN}/${dll}")
                        add_custom_command(TARGET ProjectoFinal POST_BUILD
                                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                        "${MINGW_BIN}/${dll}"
                                        $<TARGET_FILE_DIR:ProjectoFinal>
                                COMMENT "Copiando ${dll} junto al ejecutable")
                endif()
        endforeach()
endif()

add_custom_command(TARGET ProjectoFinal POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/web
                $<TARGET_FILE_DIR:ProjectoFinal>/web
        COMMENT "Copiando frontend web junto al ejecutable")
