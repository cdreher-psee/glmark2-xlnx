
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

#include <cstring>
#include <csignal>
#include <fcntl.h>
#include <libudev.h>
#include <linux/fb.h>
#include "log.h"
#include "native-state-fbdev.h"
#include <sys/ioctl.h>
#include <unistd.h>

volatile bool NativeStateFbdev::should_quit_ = false;

NativeStateFbdev::~NativeStateFbdev()
{
    if (isvalid_fd(fd_)) {
        close(fd_);
        fd_ = -1;
    }
}

bool
NativeStateFbdev::init_display()
{
    struct sigaction new_action;

    new_action.sa_handler = &NativeStateFbdev::quit_handler;
    new_action.sa_flags = 0;
    sigemptyset(&new_action.sa_mask);

    sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGTERM, &new_action, NULL);

    fd_ = get_fd();
    if (!isvalid_fd(fd_)) {
        Log::error("Failed to find a suitable FB device\n");
        return false;
    }

    return true;
}

void*
NativeStateFbdev::display()
{
    if (!isvalid_fd(fd_)) {
        Log::error("Failed to initalize display\n");
        return NULL;
    }

    return reinterpret_cast<void*>(fd_);
}

bool
NativeStateFbdev::create_window(WindowProperties const& /*properties*/)
{
    if (!isvalid_fd(fd_)) {
        Log::error("Failed to initalize display\n");
	should_quit_ = true;
        return false;
    }

    if (ioctl(fd_, FBIOGET_VSCREENINFO, &fb_info_)) {
        Log::error("Failed to get Frame buffer info\n");
	should_quit_ = true;
        return false;
    }
    return true;
}

void*
NativeStateFbdev::window(WindowProperties& properties)
{
    properties = WindowProperties(fb_info_.xres, fb_info_.yres, true, 0);

    return 0;
}

void
NativeStateFbdev::visible(bool /*visible*/)
{
}

bool
NativeStateFbdev::should_quit()
{
    return should_quit_;
}

void
NativeStateFbdev::flip()
{
}

void
NativeStateFbdev::quit_handler(int /*signum*/)
{
    should_quit_ = true;
}

bool
NativeStateFbdev::isvalid_fd(int fd)
{
    return fd >= 0;
}

int
NativeStateFbdev::get_fd()
{
    std::string node_path;
    struct fb_var_screeninfo fb_info;
    int fd = -1, temp_fd;

    Log::debug("Using Udev to detect the right fb node to use\n");
    auto udev_cntx = udev_new();
    auto dev_enum = udev_enumerate_new(udev_cntx);

    udev_enumerate_add_match_sysname(dev_enum, "fb[0-9]*");
    udev_enumerate_scan_devices(dev_enum);

    Log::debug("Looking for the right fb node...\n");

    auto entity = udev_enumerate_get_list_entry(dev_enum);

    while (entity && !isvalid_fd(fd)) {
        char const * __restrict entity_sys_path =
	    udev_list_entry_get_name(entity);

	if (entity_sys_path) {
            struct udev_device* dev =
	        udev_device_new_from_syspath(udev_cntx,
					     entity_sys_path);
	    const char * dev_node_path =
	        udev_device_get_devnode(dev);

	    temp_fd = open(dev_node_path, O_RDWR);

	    if (!ioctl(temp_fd, FBIOGET_VSCREENINFO, &fb_info)) {
                fd = temp_fd;
		break;
	    }

	    udev_device_unref(dev);
	}

	entity = udev_list_entry_get_next(entity);
    }

    return fd;
}
