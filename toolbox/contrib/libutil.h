/*
 * The Reactive C++ Toolbox.
 * Copyright (C) 2013-2019 Swirly Cloud Limited
 * Copyright (C) 2020 Reactive Markets Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef TOOLBOX_CONTRIB_LIBUTIL_H
#define TOOLBOX_CONTRIB_LIBUTIL_H

/* See https://github.com/LuaDist/libbsd */

static const char* getprogname(void) { return program_invocation_short_name; }

struct pidfh {
    int pf_fd;
    char* pf_path;
    dev_t pf_dev;
    ino_t pf_ino;
};

int flopen(const char *path, int flags, ...);
int pidfile_write(struct pidfh* pfh);
int pidfile_close(struct pidfh* pfh);
int pidfile_remove(struct pidfh* pfh);

#endif // TOOLBOX_CONTRIB_LIBUTIL_H
