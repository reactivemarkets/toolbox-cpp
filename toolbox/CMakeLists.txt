# The Reactive C++ Toolbox.
# Copyright (C) 2013-2019 Swirly Cloud Limited
# Copyright (C) 2019 Reactive Markets Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if(TOOLBOX_BUILD_SHARED)
  add_custom_target(toolbox DEPENDS
    toolbox-static
    toolbox-shared
    toolbox-test
  )
else()
  add_custom_target(toolbox DEPENDS
    toolbox-static
    toolbox-test
  )
endif()

configure_file(Config.h.in
  "${CMAKE_BINARY_DIR}/include/toolbox/Config.h" @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/include/toolbox/Config.h"
  DESTINATION include/toolbox
  COMPONENT header
)

include_directories(SYSTEM
  "${CMAKE_CURRENT_SOURCE_DIR}/contrib")

set(lib_SOURCES
  http/App.cpp
  http/Conn.cpp
  http/Error.cpp
  http/Exception.cpp
  http/Parser.cpp
  http/Request.cpp
  http/Serv.cpp
  http/Stream.cpp
  http/Types.cpp
  http/Url.cpp
  io/Buffer.cpp
  io/Epoll.cpp
  io/EpollReactor.cpp
  io/Event.cpp
  io/EventFd.cpp
  io/File.cpp
  io/Handle.cpp
  io/Interruptible.cpp
  io/Muxer.cpp
  io/Reactor.cpp
  io/Runner.cpp
  io/Timer.cpp
  io/TimerFd.cpp
  ipc/Futex.cpp
  ipc/Mmap.cpp
  ipc/MpmcQueue.cpp
  ipc/Msg.cpp
  ipc/Shm.cpp
  net/DgramSock.cpp
  net/Endpoint.cpp
  net/Error.cpp
  net/IoSock.cpp
  net/IpAddr.cpp
  net/McastSock.cpp
  net/Protocol.cpp
  net/Resolver.cpp
  net/Runner.cpp
  net/Socket.cpp
  net/StreamAcceptor.cpp
  net/StreamConnector.cpp
  net/StreamSock.cpp
  sys/Daemon.cpp
  sys/Date.cpp
  sys/Error.cpp
  sys/Limits.cpp
  sys/Log.cpp
  sys/PidFile.cpp
  sys/Signal.cpp
  sys/System.cpp
  sys/Time.cpp
  sys/Thread.cpp
  util/Argv.cpp
  util/Array.cpp
  util/Benchmark.cpp
  util/Compare.cpp
  util/Config.cpp
  util/Enum.cpp
  util/Exception.cpp
  util/Finally.cpp
  util/IntTypes.cpp
  util/Math.cpp
  util/MemAlloc.cpp
  util/Options.cpp
  util/RefCount.cpp
  util/RingBuffer.cpp
  util/Ryu.cpp
  util/Slot.cpp
  util/Stream.cpp
  util/String.cpp
  util/StringBuf.cpp
  util/Struct.cpp
  util/Tokeniser.cpp
  util/Traits.cpp
  util/TypeTraits.cpp
  util/Utility.cpp
  util/VarSub.cpp
  util/Variant.cpp
  util/Version.cpp)

add_library(toolbox-static STATIC ${lib_SOURCES})
set_target_properties(toolbox-static PROPERTIES OUTPUT_NAME toolbox)
target_link_libraries(toolbox-static pthread)
install(TARGETS toolbox-static DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT static)

if(TOOLBOX_BUILD_SHARED)
  add_library(toolbox-shared SHARED ${lib_SOURCES})
  set_target_properties(toolbox-shared PROPERTIES OUTPUT_NAME toolbox)
  target_link_libraries(toolbox-shared pthread)
  install(TARGETS toolbox-shared DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT shared)
endif()

install(FILES http.hpp DESTINATION include/toolbox COMPONENT header)
install(FILES io.hpp DESTINATION include/toolbox COMPONENT header)
install(FILES ipc.hpp DESTINATION include/toolbox COMPONENT header)
install(FILES net.hpp DESTINATION include/toolbox COMPONENT header)
install(FILES sys.hpp DESTINATION include/toolbox COMPONENT header)
install(FILES util.hpp DESTINATION include/toolbox COMPONENT header)

foreach(file ${lib_SOURCES})
  get_filename_component(dir  "${file}" DIRECTORY)
  get_filename_component(name "${file}" NAME_WE)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${name}.hpp")
    install(FILES "${dir}/${name}.hpp"
      DESTINATION "include/toolbox/${dir}"
      COMPONENT header
    )
  endif()
endforeach()

install(FILES contrib/http_parser.h
  DESTINATION include/toolbox/contrib
  COMPONENT header
)

if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU AND ${CMAKE_BUILD_TYPE} MATCHES COVERAGE)
  find_program (gcovr gcovr)
  if (gcovr)
      message (STATUS "Enabled coverage report generation target 'gcovr'")
      add_custom_target (gcovr
          COMMENT "Generating coverage report: ${CMAKE_BINARY_DIR}/gcovr/index.html"
          COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/gcovr
          COMMAND ${gcovr} --root             ${CMAKE_SOURCE_DIR}/toolbox
            --exclude          ${CMAKE_SOURCE_DIR}/toolbox/contrib
            --exclude          ${CMAKE_BINARY_DIR}
            --object-directory ${CMAKE_BINARY_DIR}
            --output           ${CMAKE_BINARY_DIR}/gcovr/index.html
            --html
            --html-details
            --html-title "ToolBox Coverage Report"
            --sort-uncovered
            --exclude-unreachable-branches
      )
  else()
      message(WARNING "gcovr not found, coverage report generation target not available")
  endif()
endif()


set(test_SOURCES
  http/Parser.ut.cpp
  http/Types.ut.cpp
  http/Url.ut.cpp
  io/Buffer.ut.cpp
  io/EpollReactor.ut.cpp
  io/Handle.ut.cpp
  io/Timer.ut.cpp
  net/Endpoint.ut.cpp
  net/IoSock.ut.cpp
  net/Resolver.ut.cpp
  net/Runner.ut.cpp
  net/Socket.ut.cpp
  sys/Date.ut.cpp
  sys/Log.ut.cpp
  sys/Time.ut.cpp
  util/Argv.ut.cpp
  util/Array.ut.cpp
  util/Config.ut.cpp
  util/Enum.ut.cpp
  util/Exception.ut.cpp
  util/Finally.ut.cpp
  util/IntTypes.ut.cpp
  util/Math.ut.cpp
  util/MemAlloc.ut.cpp
  util/Options.ut.cpp
  util/RefCount.ut.cpp
  util/RingBuffer.ut.cpp
  util/Ryu.ut.cpp
  util/Slot.ut.cpp
  util/Stream.ut.cpp
  util/String.ut.cpp
  util/StringBuf.ut.cpp
  util/Struct.ut.cpp
  util/Tokeniser.ut.cpp
  util/Traits.ut.cpp
  util/Utility.ut.cpp
  util/VarSub.ut.cpp
  util/Version.ut.cpp)

add_executable(toolbox-test
  ${test_SOURCES}
  Main.ut.cpp)
target_link_libraries(toolbox-test ${toolbox_LIBRARY} "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")

foreach(file ${test_SOURCES})
  get_filename_component(dir  "${file}" DIRECTORY)
  get_filename_component(name "${file}" NAME_WE)
  add_test(NAME ${dir}::${name}Suite COMMAND toolbox-test -l error -t ${name}Suite)
endforeach()

add_executable(toolbox-sys-bench
  sys/Time.bm.cpp
  Main.bm.cpp
)
target_link_libraries(toolbox-sys-bench ${toolbox_LIBRARY})

add_executable(toolbox-util-bench
  util/Utility.bm.cpp
  Main.bm.cpp
)
target_link_libraries(toolbox-util-bench ${toolbox_LIBRARY})
