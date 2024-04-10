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

#ifndef ORINNANO_HPP
#define ORINNANO_HPP

#include <fstream>
#include <string>

using namespace std::chrono;

namespace PMLib
{
    template <int n_lines = 4, int max_freq = 10, bool pdu = false>
    class OrinNano : public Device {
      public:
        OrinNano(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu, 
            [&] () {
	    std::vector<int> tabla = {1, 2, 3, 4};
            std::string line_voltage, line_current;
            std::vector<ifstream> files;
             std::vector<std::string> namefiles;
	    for(int i=0; i<1; i++){
	        for(int j=0; j<4; j++){
		   // Skip non-connected lines. See:
		  // https://docs.nvidia.com/jetson/archives/r35.3.1/DeveloperGuide/text/SD/PlatformPowerAndPerformance/JetsonOrinNanoSeriesJetsonOrinNxSeriesAndJetsonAgxOrinSeries.html#thermal-management-in-bpmp
		  // if ( i == 1 && ( j == 0 || j == 2 ) ) continue; 
                   files.emplace_back(get_url() + "/1-004" + std::to_string(i) + "/hwmon/hwmon0" +  "/in" + std::to_string(tabla[j]) + "_input");
                   files.emplace_back(get_url() + "/1-004" + std::to_string(i) + "/hwmon/hwmon0" +  "/curr" + std::to_string(tabla[j]) + "_input");
                   namefiles.emplace_back(get_url() + "/1-004" + std::to_string(i) + "/hwmon/hwmon0" +  "/in" + std::to_string(tabla[j]) + "_input");
                   namefiles.emplace_back(get_url() + "/1-004" + std::to_string(i) + "/hwmon/hwmon0" +  "/curr" + std::to_string(tabla[j]) + "_input");
		}
	    }	
            while ( is_running() ){
                auto t1 = system_clock::now();
                //std::cout <<"==========="<< std::endl;
		for( int i=0; i<4; i++ ){
		    
		    //std::cout << namefiles[i*2] << std::endl;
		    //std::cout << namefiles[i*2+1] << std::endl;
                    getline(files[i*2], line_voltage);
                    getline(files[i*2+1], line_current);
                    //std::cout << line_voltage << std::endl;
                    //std::cout << line_current << std::endl;
                    sample[i] = ( stof(line_voltage) * stof(line_current) ) / 1e6;
                    //std::cout << sample[i] << std::endl;
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

    static RegisterDevice< OrinNano<> > Reg_OrinNano("OrinNano");
}

#endif
