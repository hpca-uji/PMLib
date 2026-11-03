/*
 * Thor.hpp
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

#ifndef THOR_HPP
#define THOR_HPP

/// @file Thor.hpp
/// @brief Driver for the NVIDIA Jetson AGX Thor developer kit's power
///        monitors (4 lines; see settings-thor.json):
///        - lines 0-2: the three channels of the on-module INA3221
///          (VDD_GPU, VDD_CPU_SOC_MSS, VIN_SYS_5V0), each V*I from
///          {hwmon}/in{N}_input (mV) and {hwmon}/curr{N}_input (mA);
///        - line 3: total board input power, from the carrier board's
///          INA238 ({hwmon}/power1_input, in uW). It is looked up by its
///          hwmon name ("ina238") on the same I2C bus as the INA3221
///          (2-0044 on the developer kit); if there is none, the line
///          reads 0 and a warning is logged.
///        All values are in watts.
///
///        The config's "url" is the INA3221's I2C device directory, e.g.
///        /sys/bus/i2c/devices/2-0040 on the developer kit. Unlike the
///        Orin driver, the hwmonX index is not hardcoded: it is looked up
///        under {url}/hwmon/ when the device starts, because the kernel's
///        numbering changes between boards and JetPack releases. A "url"
///        that already points at a hwmon directory also works.
///
///        The rail on each channel is logged at start-up (from
///        in{N}_label), so the line names in the JSON can be checked
///        against the board.

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../utils/logger.hpp"

using namespace std::chrono;

namespace PMLib
{
    template <int n_lines = 4, int max_freq = 10, bool pdu = false>
    class Thor : public Device {

        /// Returns the hwmon directory for the INA3221 at @p url.
        static std::string find_hwmon(const std::string &url) {
            namespace fs = std::filesystem;
            if ( fs::exists(fs::path(url) / "in1_input") ) return url;
            fs::path base = fs::path(url) / "hwmon";
            if ( fs::is_directory(base) )
                for ( auto &entry : fs::directory_iterator(base) )
                    if ( entry.path().filename().string().rfind("hwmon", 0) == 0 &&
                         fs::exists(entry.path() / "in1_input") )
                        return entry.path().string();
            throw std::runtime_error("Thor: no INA3221 hwmon directory under " + url +
                                     " (expected e.g. /sys/bus/i2c/devices/2-0040)");
        }

        /// Returns the hwmon directory of the INA238 on the same I2C bus as
        /// the INA3221 hwmon directory @p ina3221, or "" if there is none.
        static std::string find_ina238(const std::string &ina3221) {
            namespace fs = std::filesystem;
            std::error_code ec;
            // {bus}/2-0040/hwmon/hwmonX -> {bus}
            fs::path bus = fs::canonical(ina3221, ec).parent_path().parent_path().parent_path();
            if ( ec || !fs::is_directory(bus) ) return "";
            for ( auto &dev : fs::directory_iterator(bus, ec) ) {
                fs::path base = dev.path() / "hwmon";
                if ( !fs::is_directory(base, ec) ) continue;
                for ( auto &hw : fs::directory_iterator(base, ec) ) {
                    std::string name;
                    std::ifstream name_file(hw.path() / "name");
                    if ( name_file && getline(name_file, name) && name == "ina238" &&
                         fs::exists(hw.path() / "power1_input") )
                        return hw.path().string();
                }
            }
            return "";
        }

      public:
        Thor(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu,
            [&] () {

            const std::string hwmon = find_hwmon(get_url());
            std::vector<ifstream> files;
            for ( int ch = 1; ch <= 3; ch++ ) {
                const std::string n = std::to_string(ch);
                std::string label = "(no label)";
                std::ifstream label_file(hwmon + "/in" + n + "_label");
                if ( label_file ) getline(label_file, label);
                CLOG_INFO << "Thor " << get_name() << ": line " << ch - 1
                          << " = INA3221 channel " << ch << " (" << label << ")" << endl;

                files.emplace_back(hwmon + "/in" + n + "_input");
                files.emplace_back(hwmon + "/curr" + n + "_input");
                if ( !files[files.size() - 2] || !files.back() )
                    throw std::runtime_error("Thor: cannot read channel " + n + " in " + hwmon);
            }

            const std::string ina238 = find_ina238(hwmon);
            ifstream total;
            if ( ina238.empty() ) {
                CLOG_WARN << "Thor " << get_name() << ": no INA238 found next to " << hwmon
                          << "; line 3 (total board power) will read 0" << endl;
            } else {
                total.open(ina238 + "/power1_input");
                CLOG_INFO << "Thor " << get_name() << ": line 3 = INA238 total input power ("
                          << ina238 << ")" << endl;
            }

            std::string line_voltage, line_current, line_power;
            while ( is_running() ){
                auto t1 = system_clock::now();

                for ( int i = 0; i < 3; i++ ){
                    getline(files[i*2], line_voltage);
                    getline(files[i*2+1], line_current);
                    sample[i] = ( stof(line_voltage) * stof(line_current) ) / 1e6;
                    files[i*2].clear();   files[i*2].seekg(0);
                    files[i*2+1].clear(); files[i*2+1].seekg(0);
                }
                if ( total.is_open() ) {
                    getline(total, line_power);
                    sample[3] = stof(line_power) / 1e6;
                    total.clear(); total.seekg(0);
                }
                yield( sample );

                auto t2 = system_clock::now();
                auto dms = duration_cast<microseconds>(t2 - t1);
                if( dms.count() < (int)(1e6/max_freq) )
                  this_thread::sleep_for(microseconds((int)(1e6/max_freq) - dms.count()));
            }

        } ) {};
    };

    /// Registers the JSON config "type": "Thor".
    static RegisterDevice< Thor<> > Reg_Thor("Thor");
}

#endif
