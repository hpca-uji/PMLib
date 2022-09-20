/*
 * Nano.hpp
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

#ifndef NANO_HPP
#define NANO_HPP

#include <fstream>
#include <string>

using namespace std::chrono;

namespace PMLib
{
    template <int n_lines = 3, int max_freq = 10, bool pdu = false>
    class Nano : public Device {
      public:
        Nano(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu, 
            [&] () {
	    
            std::string line;
            std::vector<ifstream> files;
	    for(int i=0; i<3; i++){
               files.emplace_back(get_url() + "/6-0040/iio_device/in_power" + std::to_string(i) + "_input");
	    }	
            while ( is_running() ){ 
                auto t1 = system_clock::now();

		for( int i=0; i<3; i++ ){
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

    static RegisterDevice< Nano<> > Reg_Nano("Nano");
}

#endif
