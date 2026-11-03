/*
 * device.hpp
 *
 * Created on: 27/05/2016
 *
 * =========================================================================
 *  Copyright (C) 2016-, Manuel F. Dolz (maneldz@gmail.com)
 *
 *  This file is part of PMLib.
 *
 *  PMLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PMLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this PMLib.  If not, see <http://www.gnu.org/licenses/>.
 *
 * =========================================================================
*/

#ifndef DEVICES_LIST_HPP
#define DEVICES_LIST_HPP

/// @file devices.hpp
/// @brief Aggregates every built-in device driver so pmlib_server.cpp only
///        needs one include. Each header below self-registers its
///        device type(s) with Device::create_device() via a file-scope
///        RegisterDevice instance; adding a new driver here is enough to
///        make it available from the JSON config's "type" field.

// On-board INA power monitors of embedded boards, read through sysfs/ioctl.
#include "TX2.hpp"
#include "Nano.hpp"
#include "Xavier.hpp"
#include "Orin.hpp"
#include "OrinNano.hpp"
#include "Thor.hpp"
#include "OdroidXU3.hpp"

// NVIDIA GPUs through NVML (libnvidia-ml.so, loaded at run time).
#include "NVML.hpp"

// External power meters over a serial port.
#include "LMG.hpp"
#include "ArduPower.hpp"
#include "WattsUp.hpp"

// Fake device for testing without hardware.
#include "Dummy.hpp"

// AccelPower CAPE (needs Linux I2C / libi2c, so it is opt-in).
#ifdef USE_DEVICE_APCAPE
#include "APCape.hpp"
#endif

#endif
