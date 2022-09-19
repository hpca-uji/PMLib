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

#include <fstream>
#include <string>

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
	        sample[0]++;  
		for( int i=0; i<4; i++ ){
                    getline(files[i*2], line_voltage);
                    getline(files[i*2+1], line_current);
                    sample[i] = ( stof(line_voltage) * stof(line_current) ) / 1e6;
                    files[i*2].seekg(0);
                    files[i*2+1].seekg(0);
		}
                yield( sample );
                this_thread::sleep_for(std::chrono::microseconds((int)(1e6/max_freq)));
            }

        } ) {};   
    };

    static RegisterDevice< Orin<> > Reg_Orin("Orin");
}

#endif
