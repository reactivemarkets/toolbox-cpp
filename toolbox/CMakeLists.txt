# The Reactive C++ Toolbox.
# Copyright (C) 2013-2019 Swirly Cloud Limited
# Copyright (C) 2020 Reactive Markets Limited
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
  add_custom_target(tb-core DEPENDS
    tb-core-static
    tb-core-shared
    tb-core-test
  )
else()
  add_custom_target(tb-core DEPENDS
    tb-core-static
    tb-core-test
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
  hdr/Histogram.cpp
  hdr/Iterator.cpp
  hdr/Utility.cpp
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
  io/Disposer.cpp
  io/Epoll.cpp
  io/Event.cpp
  io/EventFd.cpp
  io/File.cpp
  io/Handle.cpp
  io/Hook.cpp
  io/Waker.cpp
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
  net/Endian.cpp
  net/Endpoint.cpp
  net/Error.cpp
  net/Frame.cpp
  net/IoSock.cpp
  net/IpAddr.cpp
  net/McastSock.cpp
  net/Protocol.cpp
  net/RateLimit.cpp
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
  sys/Thread.cpp
  sys/Time.cpp
  sys/Trace.cpp
  util/Alarm.cpp
  util/Argv.cpp
  util/Array.cpp
  util/Bits.cpp
  util/Config.cpp
  util/Enum.cpp
  util/Exception.cpp
  util/Finally.cpp
  util/IntTypes.cpp
  util/Math.cpp
  util/MemAlloc.cpp
  util/Options.cpp
  util/Options.cpp
  util/Path.cpp
  util/RingBuffer.cpp
  util/RobinHood.cpp
  util/Ryu.cpp
  util/Slot.cpp
  util/Stream.cpp
  util/String.cpp
  util/StringBuf.cpp
  util/Struct.cpp
  util/Tokeniser.cpp
  util/Traits.cpp
  util/Tuple.cpp
  util/TypeTraits.cpp
  util/Utility.cpp
  util/VarSub.cpp
  util/Variant.cpp
  util/Version.cpp)

add_library(tb-core-static STATIC ${lib_SOURCES})
set_target_properties(tb-core-static PROPERTIES OUTPUT_NAME tb-core)
target_link_libraries(tb-core-static pthread)
install(TARGETS tb-core-static DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT static)

if(TOOLBOX_BUILD_SHARED)
  add_library(tb-core-shared SHARED ${lib_SOURCES})
  set_target_properties(tb-core-shared PROPERTIES OUTPUT_NAME tb-core)
  target_link_libraries(tb-core-shared pthread)
  install(TARGETS tb-core-shared DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT shared)
endif()

set(lib_bm_SOURCES
  bm/Benchmark.cpp
  bm/Ctx.cpp
  bm/Range.cpp
  bm/Record.cpp
  bm/Suite.cpp
  bm/Utility.cpp)

add_library(tb-bm-static STATIC ${lib_bm_SOURCES})
set_target_properties(tb-bm-static PROPERTIES OUTPUT_NAME tb-bm)
target_link_libraries(tb-bm-static ${tb_core_LIBRARY})
install(TARGETS tb-bm-static DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT static)

if(TOOLBOX_BUILD_SHARED)
  add_library(tb-bm-shared SHARED ${lib_bm_SOURCES})
  set_target_properties(tb-bm-shared PROPERTIES OUTPUT_NAME tb-bm)
  target_link_libraries(tb-bm-shared ${tb_core_LIBRARY})
  install(TARGETS tb-bm-shared DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT shared)
endif()

install(FILES bm.hpp DESTINATION include/toolbox COMPONENT header)
install(FILES hdr.hpp DESTINATION include/toolbox COMPONENT header)
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
  hdr/Histogram.ut.cpp
  hdr/Iterator.ut.cpp
  hdr/Utility.ut.cpp
  http/Parser.ut.cpp
  http/Types.ut.cpp
  http/Url.ut.cpp
  io/Buffer.ut.cpp
  io/Disposer.ut.cpp
  io/Handle.ut.cpp
  io/Hook.ut.cpp
  io/Reactor.ut.cpp
  io/Timer.ut.cpp
  net/Endpoint.ut.cpp
  net/Frame.ut.cpp
  net/IoSock.ut.cpp
  net/RateLimit.ut.cpp
  net/Resolver.ut.cpp
  net/Runner.ut.cpp
  net/Socket.ut.cpp
  sys/Date.ut.cpp
  sys/Log.ut.cpp
  sys/Thread.ut.cpp
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
  util/Path.ut.cpp
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
  util/Tuple.ut.cpp
  util/Utility.ut.cpp
  util/VarSub.ut.cpp
  util/Version.ut.cpp)

add_executable(tb-core-test
  ${test_SOURCES}
  Main.ut.cpp)
target_link_libraries(tb-core-test
  ${tb_core_LIBRARY} "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}" stdc++fs)

foreach(file ${test_SOURCES})
  get_filename_component(dir  "${file}" DIRECTORY)
  get_filename_component(name "${file}" NAME_WE)
  add_test(NAME ${dir}::${name}Suite COMMAND tb-core-test -l error -t ${name}Suite)
endforeach()
