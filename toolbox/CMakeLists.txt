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
  io/Stream.cpp
  io/Timer.cpp
  io/TimerFd.cpp
  ipc/Futex.cpp
  ipc/Mmap.cpp
  ipc/MpmcQueue.cpp
  ipc/Msg.cpp
  ipc/Shm.cpp
  ipc/Signal.cpp
  net/DgramSock.cpp
  net/Endpoint.cpp
  net/Error.cpp
  net/IoSock.cpp
  net/IpAddr.cpp
  net/McastSock.cpp
  net/Protocol.cpp
  net/Resolver.cpp
  net/Socket.cpp
  net/StreamAcceptor.cpp
  net/StreamConnector.cpp
  net/StreamSock.cpp
  sys/Argv.cpp
  sys/Daemon.cpp
  sys/Date.cpp
  sys/Error.cpp
  sys/Limits.cpp
  sys/Log.cpp
  sys/Options.cpp
  sys/PidFile.cpp
  sys/System.cpp
  sys/Time.cpp
  util/Array.cpp
  util/Compare.cpp
  util/Config.cpp
  util/Enum.cpp
  util/Exception.cpp
  util/Finally.cpp
  util/IntTypes.cpp
  util/Math.cpp
  util/RefCount.cpp
  util/RingBuffer.cpp
  util/Ryu.cpp
  util/Slot.cpp
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

set(test_SOURCES
  io/Buffer.ut.cpp
  io/EpollReactor.ut.cpp
  io/Handle.ut.cpp
  io/Stream.ut.cpp
  io/Timer.ut.cpp
  net/Endpoint.ut.cpp
  net/IoSock.ut.cpp
  net/Resolver.ut.cpp
  net/Socket.ut.cpp
  sys/Argv.ut.cpp
  sys/Date.ut.cpp
  sys/Log.ut.cpp
  sys/Options.ut.cpp
  sys/Time.ut.cpp
  util/Array.ut.cpp
  util/Config.ut.cpp
  util/Enum.ut.cpp
  util/Exception.ut.cpp
  util/Finally.ut.cpp
  util/IntTypes.ut.cpp
  util/Math.ut.cpp
  util/RefCount.ut.cpp
  util/RingBuffer.ut.cpp
  util/Ryu.ut.cpp
  util/Slot.ut.cpp
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
