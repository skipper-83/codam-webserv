cmake_minimum_required(VERSION 3.21.0)

project(webserv)

# Sourcefiles with main function (excluded from testing)
set (SRCS_MAIN
	src/main.cpp
)

# Sourcefiles without main function (included in testing)
set (SRCS
	src/logging.cpp
	src/async/fd.cpp
	src/async/input.cpp
	src/async/output.cpp
	src/async/io.cpp
	src/async/socket.cpp
	src/async/socket_client.cpp
	src/async/in_file.cpp
	src/async/pollarray.cpp
	src/async/program.cpp
	src/file_handler.cpp
	src/client/client.cpp
	src/client/client_callbacks.cpp
	src/client/client_util.cpp
	src/client/client_file_handling.cpp
	src/client/client_cgi_handling.cpp
	src/client/client_session.cpp
	src/client/client_build_response.cpp
	src/config/config_extractors.cpp
	src/config/config_insertions.cpp
	src/config/config.cpp
	src/httpMessage/httpRequest/http_request.cpp
	src/httpMessage/httpRequest/parse.cpp
	src/httpMessage/httpRequest/parse_header.cpp
	src/httpMessage/httpRequest/parse_body.cpp
	src/httpMessage/httpRequest/getters_setters.cpp
	src/httpMessage/httpRequest/helpers.cpp
	src/httpMessage/http_message.cpp
	src/httpMessage/http_response.cpp
	src/httpMessage/cgi_message.cpp
	src/session.cpp
	src/util.cpp
	src/buffer.cpp
)
set (EXEC_NAME webserv)

# Compiler settings
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "-Wall -Werror -Wextra")
set (CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=address") #cmake -DCMAKE_BUILD_TYPE=DEBUG to add debug flags
set (CMAKE_EXPORT_COMPILE_COMMANDS ON) #vscode include path checking

# Make executable with output name specified under EXEC_NAME and sources specified under SRCS and SRCS_MAIN
add_executable(${EXEC_NAME} ${SRCS} ${SRCS_MAIN})
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link libraries
add_subdirectory(lib/cpplog)
target_link_libraries(${EXEC_NAME} cpplog)
# target_link_libraries(ava_test cpplog)

