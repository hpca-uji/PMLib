/*
 * TX2.hpp
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

#ifndef TX2_HPP
#define TX2_HPP

/// @file TX2.hpp
/// @brief Driver for the NVIDIA Jetson TX2's four on-board INA3221 power
///        monitors (12 lines, 3 rails each; see settings-tx2.json for the
///        rail names). Reads instantaneous power (mW) from
///        <url>/0-004{0..3}/iio:device{0..3}/in_power{0..2}_input and
///        reports watts. The config's "url" is the I2C bus directory, e.g.
///        /sys/devices/3160000.i2c/i2c-0/.

#include <fstream>
#include <string>

using namespace std::chrono;

namespace PMLib
{
    template <int n_lines = 12, int max_freq = 10, bool pdu = false>
    class TX2 : public Device {
      public:
        TX2(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu,
            [&] () {

            std::string line;
            std::vector<ifstream> files;
            for(int i=0; i<4; i++){
                for(int j=0; j<3; j++){
                   files.emplace_back(get_url() + "/0-004" + std::to_string(i) + "/iio:device" + std::to_string(i) + "/in_power" + std::to_string(j) + "_input");
                }
            }
            while ( is_running() ){
                auto t1 = system_clock::now();

                for( int i=0; i<12; i++ ){
                    getline(files[i],line);
                    sample[i] = stof(line) / 1e3;
                    files[i].seekg(0);
                }
                yield( sample );

                auto t2 = system_clock::now();
                auto dms = duration_cast<microseconds>(t2 - t1);
                if( dms.count() < (int)(1e6/max_freq) )
                  this_thread::sleep_for(microseconds((int)(1e6/max_freq) - dms.count()));
            }

        } ) {};
    };

    /// Registers the JSON config "type": "TX2".
    static RegisterDevice< TX2<> > Reg_TX2("TX2");
}

#endif
