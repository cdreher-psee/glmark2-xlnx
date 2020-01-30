/*
 * Copyright © 2018 Xilinx Inc
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Madhurkiran Harikrishnan <madhurkiran.harikrishnan@xilinx.com>
 */

#ifndef GLMARK2_NATIVE_STATE_FBDEV_H_
#define GLMARK2_NATIVE_STATE_FBDEV_H_

#include <csignal>
#include <cstring>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <linux/fb.h>
#include "native-state.h"

class NativeStateFbdev : public NativeState
{
public:
    NativeStateFbdev() :
        fd_(-1),
        native_window_((EGLNativeWindowType)NULL) {}
    ~NativeStateFbdev();

    bool init_display();
    void* display();
    bool create_window(WindowProperties const& properties);
    void* window(WindowProperties& properties);
    void visible(bool v);
    bool should_quit();
    void flip();

private:
    int fd_;
    struct fb_var_screeninfo fb_info_;
    NativeWindowType native_window_;
    static volatile bool should_quit_;
    static void quit_handler(int signum);
    static bool isvalid_fd(int fd);
    static int get_fd();
};

#endif /* GLMARK2_NATIVE_STATE_FBDEV_H_ */
