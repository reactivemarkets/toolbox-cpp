// The Reactive C++ Toolbox.
// Copyright (C) 2020 Reactive Markets Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TOOLBOX_SYS_TRACE_HPP
#define TOOLBOX_SYS_TRACE_HPP

#if TOOLBOX_HAVE_SYSTEMTAP
#ifndef SDT_USE_VARIADIC
#define SDT_USE_VARIADIC
#endif
#include <sys/sdt.h>
#define TOOLBOX_PROBE(provider, name, ...) STAP_PROBEV(provider, name, ##__VA_ARGS__)
#else
#define TOOLBOX_PROBE(provider, name, ...)
#endif

#endif // TOOLBOX_SYS_TRACE_HPP
