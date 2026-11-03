/*
 * NVML.hpp
 *
 * =========================================================================
 *  Copyright (C) 2016-, Manuel F. Dolz (dolzm@uji.es)
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

#ifndef NVML_HPP
#define NVML_HPP

/// @file NVML.hpp
/// @brief Driver for the power draw of an NVIDIA GPU as reported by NVML,
///        the library behind nvidia-smi (1 line, in watts). It is the same
///        value as `nvidia-smi --query-gpu=power.draw`, and works with any
///        NVIDIA GPU that nvidia-smi sees.
///
///        The config's "url" is the GPU index as nvidia-smi numbers it
///        ("0" for the first or only GPU). For several GPUs, declare one
///        device per GPU (see settings-nvml.json); settings-dgx-spark.json
///        is the single-GPU case.
///
///        libnvidia-ml.so.1 is loaded with dlopen() when the device starts,
///        so the server builds without CUDA or NVML installed and only
///        needs the NVIDIA driver on the machine where this device is used.
///
///        On the DGX Spark (GB10) this is the GPU only: NVIDIA exposes no
///        CPU or whole-system power sensor there, so use an external meter
///        (e.g. LMG450) on the power supply to measure the whole machine.

#include <dlfcn.h>
#include <stdexcept>
#include <string>

#include "../utils/logger.hpp"

using namespace std::chrono;

namespace PMLib
{
    template <int n_lines = 1, int max_freq = 10, bool pdu = false>
    class NVML : public Device {

        // The subset of nvml.h used here, declared locally so that the
        // header (part of the CUDA toolkit) is not needed to build.
        using nvmlReturn_t = int;          // NVML_SUCCESS == 0
        using nvmlDevice_t = void *;
        using init_t       = nvmlReturn_t (*)();
        using shutdown_t   = nvmlReturn_t (*)();
        using handle_t     = nvmlReturn_t (*)(unsigned int, nvmlDevice_t *);
        using name_t       = nvmlReturn_t (*)(nvmlDevice_t, char *, unsigned int);
        using power_t      = nvmlReturn_t (*)(nvmlDevice_t, unsigned int *);
        using error_t      = const char * (*)(nvmlReturn_t);

        template <typename F>
        static F symbol(void *lib, const char *name) {
            F f = reinterpret_cast<F>(dlsym(lib, name));
            if ( !f ) throw std::runtime_error(std::string("NVML: missing symbol ") + name);
            return f;
        }

      public:
        NVML(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu,
            [&] () {

            void *lib = dlopen("libnvidia-ml.so.1", RTLD_NOW);
            if ( !lib ) lib = dlopen("libnvidia-ml.so", RTLD_NOW);
            if ( !lib )
                throw std::runtime_error(std::string("NVML: cannot load libnvidia-ml.so.1 "
                                         "(is the NVIDIA driver installed?): ") + dlerror());

            auto nvml_init     = symbol<init_t>(lib, "nvmlInit_v2");
            auto nvml_shutdown = symbol<shutdown_t>(lib, "nvmlShutdown");
            auto nvml_handle   = symbol<handle_t>(lib, "nvmlDeviceGetHandleByIndex_v2");
            auto nvml_name     = symbol<name_t>(lib, "nvmlDeviceGetName");
            auto nvml_power    = symbol<power_t>(lib, "nvmlDeviceGetPowerUsage");
            auto nvml_error    = symbol<error_t>(lib, "nvmlErrorString");

            auto check = [&] (nvmlReturn_t r, const char *what) {
                if ( r != 0 ) throw std::runtime_error(std::string("NVML: ") + what + ": " + nvml_error(r));
            };

            unsigned int index;
            try { index = std::stoul(get_url()); }
            catch ( const std::exception& ) {
                throw std::runtime_error("NVML: \"url\" must be a GPU index such as \"0\", got \"" + get_url() + "\"");
            }

            check(nvml_init(), "nvmlInit");
            nvmlDevice_t dev;
            check(nvml_handle(index, &dev), "nvmlDeviceGetHandleByIndex");
            char gpu_name[96] = "";
            nvml_name(dev, gpu_name, sizeof(gpu_name));
            CLOG_INFO << "NVML " << get_name() << ": line 0 = power draw of GPU "
                      << index << " (" << gpu_name << ")" << endl;

            unsigned int milliwatts;
            while ( is_running() ){
                auto t1 = system_clock::now();

                check(nvml_power(dev, &milliwatts), "nvmlDeviceGetPowerUsage");
                sample[0] = milliwatts / 1e3;
                yield( sample );

                auto t2 = system_clock::now();
                auto dms = duration_cast<microseconds>(t2 - t1);
                if( dms.count() < (int)(1e6/max_freq) )
                  this_thread::sleep_for(microseconds((int)(1e6/max_freq) - dms.count()));
            }

            nvml_shutdown();
            dlclose(lib);

        } ) {};
    };

    /// Registers the JSON config "type": "NVML".
    static RegisterDevice< NVML<> > Reg_NVML("NVML");
}

#endif
