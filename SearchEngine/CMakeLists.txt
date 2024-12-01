cmake_minimum_required(VERSION 3.16)
project(SpiderSearchEngine)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(_WIN32_WINNT=0x0601)

# Пути к исходным файлам
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")

# Альтернативные пути к библиотекам
set(EXT_OPENSSL_DIR "C:/Program Files/OpenSSL-Win64")
set(EXT_BOOST_DIR "C:/Program Files/boost/boost_1_86_0")

# Исходные файлы основного проекта
set(SOURCES
    ${SRC_DIR}/main.cpp

    ${SRC_DIR}/Database/Database.cpp

    ${SRC_DIR}/Indexer/Indexer.cpp

    ${SRC_DIR}/Utils/Config.cpp
    ${SRC_DIR}/Utils/URLParser.cpp
    ${SRC_DIR}/Utils/HTTPUtils.cpp
    ${SRC_DIR}/Utils/Logger.cpp

    ${SRC_DIR}/Spider/Spider.cpp

    ${SRC_DIR}/SearchEngine/SearchEngine.cpp

    ${SRC_DIR}/Application/Application.cpp
)

# Заголовочные файлы
set(HEADERS
    ${SRC_DIR}/Database/Database.h

    ${SRC_DIR}/Indexer/Indexer.h

    ${SRC_DIR}/Utils/Config.h
    ${SRC_DIR}/Utils/URLParser.h
    ${SRC_DIR}/Utils/HTTPUtils.h
    ${SRC_DIR}/Utils/Logger.h

    ${SRC_DIR}/Spider/Spider.h

    ${SRC_DIR}/SearchEngine/SearchEngine.h

    ${SRC_DIR}/Application/Application.h
)

# Исходные файлы только поисковика
set(SEARCH_SOURCES
    ${SRC_DIR}/main_search.cpp

    ${SRC_DIR}/Database/Database.cpp

    ${SRC_DIR}/Indexer/Indexer.cpp

    ${SRC_DIR}/Utils/Config.cpp
    ${SRC_DIR}/Utils/URLParser.cpp
    ${SRC_DIR}/Utils/HTTPUtils.cpp
    ${SRC_DIR}/Utils/Logger.cpp

    ${SRC_DIR}/Spider/Spider.cpp

    ${SRC_DIR}/SearchEngine/SearchEngine.cpp

    ${SRC_DIR}/Application/Application.cpp
)

# Заголовочные файлы
set(SEARCH_HEADERS
    ${SRC_DIR}/Database/Database.h

    ${SRC_DIR}/Indexer/Indexer.h

    ${SRC_DIR}/Utils/Config.h
    ${SRC_DIR}/Utils/URLParser.h
    ${SRC_DIR}/Utils/HTTPUtils.h
    ${SRC_DIR}/Utils/Logger.h

    ${SRC_DIR}/Spider/Spider.h

    ${SRC_DIR}/SearchEngine/SearchEngine.h

    ${SRC_DIR}/Application/Application.h
)

add_executable(SpiderSearchEngine ${SOURCES} ${HEADERS})
target_compile_definitions(SpiderSearchEngine PRIVATE FULL_PROJECT_MODE)
add_executable(SearchOnly         ${SEARCH_SOURCES} ${SEARCH_HEADERS})

# ------------------------------------------------------------------------
# Политика CMP0144 для подавления предупреждений
# ------------------------------------------------------------------------
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

# -----------------------------------------------------------------------------
# Подключение libpqxx
# -----------------------------------------------------------------------------
set (LIBPQXX_DIR "${LIBS_DIR}/libpqxx-7.9.2")
add_subdirectory(${LIBPQXX_DIR} build_libpqxx)

target_link_libraries(SpiderSearchEngine PRIVATE pqxx)
target_link_libraries(SearchOnly PRIVATE pqxx)

# -----------------------------------------------------------------------------
# Подключение Boost
# -----------------------------------------------------------------------------
set(BOOST_ROOT ${EXT_BOOST_DIR})
set(BOOST_LIBRARYDIR "${EXT_BOOST_DIR}/stage/lib")

find_package(Boost REQUIRED)

if (Boost_FOUND)
    target_include_directories(SpiderSearchEngine PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_directories(SpiderSearchEngine PRIVATE ${BOOST_LIBRARYDIR})

    target_include_directories(SearchOnly PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_directories(SearchOnly PRIVATE ${BOOST_LIBRARYDIR})
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# -----------------------------------------------------------------------------
# Подключение OpenSSL
# -----------------------------------------------------------------------------
find_package(OpenSSL REQUIRED)

if (OpenSSL_FOUND)
    target_include_directories(SpiderSearchEngine PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(SpiderSearchEngine PRIVATE ${OPENSSL_LIBRARIES})
    
    target_include_directories(SearchOnly PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(SearchOnly PRIVATE ${OPENSSL_LIBRARIES})
else()
    set(OPENSSL_INCLUDE_DIR "${EXT_OPENSSL_DIR}/include")
    set(OPENSSL_CRYPTO_LIBRARY "${EXT_OPENSSL_DIR}/lib/VC/x64/MDd/libcrypto.lib")
    set(OPENSSL_SSL_LIBRARY "${EXT_OPENSSL_DIR}/lib/VC/x64/MDd/libssl.lib")
    target_include_directories(SpiderSearchEngine PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(SpiderSearchEngine PRIVATE ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})
    
    target_include_directories(SearchOnly PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(SearchOnly PRIVATE ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})
endif()

# -----------------------------------------------------------------------------
# Копируем Config к билду проекта
# -----------------------------------------------------------------------------
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/Config.ini")
set(CONFIG_DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_custom_command(
    TARGET SpiderSearchEngine POST_BUILD                            # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different                   # which executes "cmake - E copy_if_different..."
        ${CONFIG_FILE} $<TARGET_FILE_DIR:SpiderSearchEngine>                       
)

# -----------------------------------------------------------------------------
# Вывод путей для отладки
# -----------------------------------------------------------------------------
message(STATUS "Found Boost: ${Boost_VERSION}")
message(STATUS "Boost Include Dir: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost Library Dir: ${BOOST_LIBRARYDIR}")

message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
message(STATUS "OpenSSL Include Dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")

message(STATUS "Libpqxx Library Dir: ${LIBS_DIR}/libpqxx-7.9.2")