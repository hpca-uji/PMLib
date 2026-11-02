/*
 * Orin.hpp
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

#ifndef ORIN_HPP
#define ORIN_HPP

/// @file Orin.hpp
/// @brief Driver for the NVIDIA Jetson AGX Orin's two on-board INA3221
///        power monitors (4 lines; see settings-orin.json). Channels 1 and
///        3 of the second monitor are not connected on this board and are
///        skipped. Each sample is V*I from
///        {url}/1-004{0,1}/hwmon/hwmon{1,2}/{in,curr}N_input, in watts; the
///        hwmon indices are hardcoded to the kernel's enumeration on the
///        boards this was written for. The config's "url" is the INA3221
///        driver directory, e.g. /sys/bus/i2c/drivers/ina3221/.

#include <fstream>
#include <string>

using namespace std::chrono;

namespace PMLib
{
    template <int n_lines = 4, int max_freq = 10, bool pdu = false>
    class Orin : public Device {
      public:
        Orin(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu,
            [&] () {

            std::string line_voltage, line_current;
            std::vector<ifstream> files;
            for(int i=0; i<2; i++){
                for(int j=0; j<3; j++){
                   // Skip non-connected lines. See:
                   // https://docs.nvidia.com/jetson/archives/r34.1/DeveloperGuide/text/SD/PlatformPowerAndPerformance/JetsonOrinNxSeriesAndJetsonAgxOrinSeries.html#software-based-power-consumption-modeling
                   if ( i == 1 && ( j == 0 || j == 2 ) ) continue;
                   files.emplace_back(get_url() + "/1-004" + std::to_string(i) + "/hwmon/hwmon" + std::to_string(i+1) + "/in" + std::to_string(j+1) + "_input");
                   files.emplace_back(get_url() + "/1-004" + std::to_string(i) + "/hwmon/hwmon" + std::to_string(i+1) + "/curr" + std::to_string(j+1) + "_input");
                }
            }
            while ( is_running() ){
                auto t1 = system_clock::now();

                for( int i=0; i<4; i++ ){
                    getline(files[i*2], line_voltage);
                    getline(files[i*2+1], line_current);
                    sample[i] = ( stof(line_voltage) * stof(line_current) ) / 1e6;
                    files[i*2].seekg(0);
                    files[i*2+1].seekg(0);
                }
                yield( sample );

                auto t2 = system_clock::now();
                auto dms = duration_cast<microseconds>(t2 - t1);
                if( dms.count() < (int)(1e6/max_freq) )
                  this_thread::sleep_for(microseconds((int)(1e6/max_freq) - dms.count()));
            }

        } ) {};
    };

    /// Registers the JSON config "type": "Orin".
    static RegisterDevice< Orin<> > Reg_Orin("Orin");
}

#endif
