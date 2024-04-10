/*
 * OdroidXU3.hpp
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

#ifndef ODROIDXU3_HPP
#define ODROIDXU3_HPP

#include <chrono>
#include <iostream>
#include <memory>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

using namespace std::chrono;

#define GPUFREQ_NODE    "/sys/devices/11800000.mali/clock"
#define TEMP_NODE       "/sys/devices/10060000.tmu/temp"

using namespace std;

typedef struct ina231_iocreg__t {
    char name[20];
    unsigned int enable;
    unsigned int cur_uV;
    unsigned int cur_uA;
    unsigned int cur_uW;
} ina231_iocreg_t;

typedef struct sensor__t {
    int  fd;
    ina231_iocreg_t data;
} sensor_t;

#define INA231_IOCGREG      _IOR('i', 1, ina231_iocreg_t *)
#define INA231_IOCSSTATUS   _IOW('i', 2, ina231_iocreg_t *)
#define INA231_IOCGSTATUS   _IOR('i', 3, ina231_iocreg_t *)

#define DEV_SENSOR_ARM  "/dev/sensor_arm"
#define DEV_SENSOR_MEM  "/dev/sensor_mem"
#define DEV_SENSOR_KFC  "/dev/sensor_kfc"
#define DEV_SENSOR_G3D  "/dev/sensor_g3d"

enum    {
    SENSOR_ARM = 0,
    SENSOR_MEM,
    SENSOR_KFC,
    SENSOR_G3D,
    SENSOR_MAX
};


class GetNode
{
public:
    GetNode() {
      for (int i = 0; i < 8; i++) {
        string temp("/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/cpuinfo_cur_freq");
        cpu_node_list[i] = temp;
      }
    };

    string cpu_node_list[8];
    float armuV,armuA, armuW;
    float g3duV, g3duA, g3duW;
    float kfcuV, kfcuA, kfcuW;
    float memuV, memuA, memuW;
    int usage[8]   = {0};
    int cpuFreq[8] = {0};
    int cpuTemp[4] = {0};
    int gpuFreq    = 0;
    int gpuTemp    = 0;

    int GetGPUCurFreq(void) {
      std::ifstream fp;
      fp.open(GPUFREQ_NODE);
    
      if(fp.fail()) {
        std::cerr << "GPUFRE_NODE open failed" << endl;
        return -1;
      }
    
      string freq;
      std::getline(fp, freq);
      fp.close();
      gpuFreq = std::stoi(freq);
    
      return 0;
    };

    int GetCPUCurFreq(int cpuNum) {
      if(cpuNum > 7) {
        return -1;
      }
      else if(cpuNum >= 0) {
        cpuFreq[cpuNum] = GetOneCPUCurFreq(cpuNum);
        return 0;
      }
      else {
        for(int i=0; i<8; i++) {
          cpuFreq[i] = GetOneCPUCurFreq(i);
        }
        return 0;
      }
    };

    int GetOneCPUCurFreq(int cpuNum) {
      std::ifstream fp;
      fp.open(cpu_node_list[cpuNum]);
    
      if(fp.fail()) {
        std::cerr << "CPU " << cpuNum << " open failed: " << cpu_node_list[cpuNum] << endl;
        return -1;
      }
    
      string freq;
      std::getline(fp, freq);
      fp.close();
      int freqi = std::stoi(freq)/1000;
    
      return freqi;
    };

    int GetOneCPUTemp(int cpuNum) {
      std::ifstream fp;
      fp.open(TEMP_NODE);
    
      if(fp.fail())
        return -1;
    
      char buf[16];
      for (int i=0; i<cpuNum+1; i++)
        fp.read(buf, 16);
      fp.close();
    
      buf[12] = '\0';
    
      return atoi(&buf[9]);
    };

    int GetCPUTemp(int cpuNum) {
      if(cpuNum > 3) {
        return -1;
      }
      else if(cpuNum <0) {
        for(int i=0; i<4; i++) {
          cpuTemp[i] = GetOneCPUTemp(i);
        }
      }
      else {
        cpuTemp[cpuNum] = GetOneCPUTemp(cpuNum);
      }
    
      return 0;
    };
    
    int GetGPUTemp() {
      std::ifstream fp;
      fp.open(TEMP_NODE);
    
      if(fp.fail())
        return -1;
    
      char buf[16];
      for (int i=0; i<5; i++)
        fp.read(buf, 16);
      fp.close();
    
      buf[12] = '\0';
    
      gpuTemp = atoi(&buf[9]);
    
      return 0;
    };

    int GetCPUUsage(void) {
      char buf[128] = {0,};
      char cpuid[8] = "cpu";
      int findCPU = 0;
      int user, system, nice, idle;
      FILE *fp;
      int cpu_index = 0;
    
      fp = fopen("/proc/stat", "r");
      if (fp == NULL)
        return -1;
    
      while (fgets(buf, 128, fp)) {
        char temp[4] = "cpu";
        temp[3] = '0' + cpu_index;
        if (!strncmp(buf, temp, 4)) {
          findCPU = 1;
          sscanf(buf, "%s %d %d %d %d",
                 cpuid, &user, &nice, &system, &idle);
    
          usage[cpu_index] = calUsage(cpu_index, user, nice, system, idle);
    
          cpu_index++;
        }
        if (!strncmp(buf, "intr", 4))
          break;
        if (findCPU == 0) {
    
        }
        else
          findCPU = 0;
      }
      fclose(fp);
    
      return 0;
    };

    int calUsage(int cpu_idx, int user, int nice, int system, int idle) {
      long total = 0;
      long usage = 0;
      int diff_user, diff_system, diff_idle;
    
      diff_user = mOldUserCPU[cpu_idx] - user;
      diff_system = mOldSystemCPU[cpu_idx] - system;
      diff_idle = mOldIdleCPU[cpu_idx] - idle;
    
      total = diff_user + diff_system + diff_idle;
    
      if (total != 0)
        usage = (diff_user * 100) / total;
    
      mOldUserCPU[cpu_idx] = user;
      mOldSystemCPU[cpu_idx] = system;
      mOldIdleCPU[cpu_idx] = idle;
    
      return usage;
    };

    int open_sensor(const char *node, sensor_t *sensor) {
      if ((sensor->fd = open(node, O_RDWR)) < 0) {}
      //   qDebug() << node << "Open Fail";
    
      return sensor->fd;
    };

    void close_sensor(sensor_t *sensor) {
      if (sensor->fd > 0)
        close(sensor->fd);
    };

    void enable_sensor(sensor_t *sensor, unsigned char enable) {
      if (sensor->fd > 0) {
        sensor->data.enable = enable ? 1 : 0;
        if (ioctl(sensor->fd, INA231_IOCSSTATUS, &sensor->data) < 0){}
        //qDebug() << "IOCTL Error";
      }
    };

    int read_sensor_status(sensor_t *sensor) {
      if (sensor->fd > 0) {
        if (ioctl(sensor->fd, INA231_IOCGSTATUS, &sensor->data) < 0) {
          std::cerr << "Read Sensor Status IOCTL Error!" << endl;
          return -1;
        }
      }
      return 0;
    };

    int read_sensor(sensor_t *sensor) {
      if (sensor->fd > 0) {
        if (ioctl(sensor->fd, INA231_IOCGREG, &sensor->data) < 0) {
          std::cerr << "Read Sensor IOCTL Error!" << endl;
          return -1;
        }
      }
    
      return 0;
    };

    int OpenINA231(void) {
      if (open_sensor(DEV_SENSOR_ARM, &sensor[SENSOR_ARM]) < 0)
        return -1;
      if (open_sensor(DEV_SENSOR_MEM, &sensor[SENSOR_MEM]) < 0)
        return -1;
      if (open_sensor(DEV_SENSOR_KFC, &sensor[SENSOR_KFC]) < 0)
        return -1;
      if (open_sensor(DEV_SENSOR_G3D, &sensor[SENSOR_G3D]) < 0)
        return -1;
    
      if (read_sensor_status(&sensor[SENSOR_ARM]))
        return -1;
      if (read_sensor_status(&sensor[SENSOR_MEM]))
        return -1;
      if (read_sensor_status(&sensor[SENSOR_KFC]))
        return -1;
      if (read_sensor_status(&sensor[SENSOR_G3D]))
        return -1;
    
      if (!sensor[SENSOR_ARM].data.enable)
        enable_sensor(&sensor[SENSOR_ARM], 1);
      if (!sensor[SENSOR_MEM].data.enable)
        enable_sensor(&sensor[SENSOR_MEM], 1);
      if (!sensor[SENSOR_KFC].data.enable)
        enable_sensor(&sensor[SENSOR_KFC], 1);
      if (!sensor[SENSOR_G3D].data.enable)
        enable_sensor(&sensor[SENSOR_G3D], 1);
    
      return 0;
    };

    void CloseINA231(void) {
      if (sensor[SENSOR_ARM].data.enable)
        enable_sensor(&sensor[SENSOR_ARM], 0);
      if (sensor[SENSOR_MEM].data.enable)
        enable_sensor(&sensor[SENSOR_MEM], 0);
      if (sensor[SENSOR_KFC].data.enable)
        enable_sensor(&sensor[SENSOR_KFC], 0);
      if (sensor[SENSOR_G3D].data.enable)
        enable_sensor(&sensor[SENSOR_G3D], 0);
    
      close_sensor(&sensor[SENSOR_ARM]);
      close_sensor(&sensor[SENSOR_MEM]);
      close_sensor(&sensor[SENSOR_KFC]);
      close_sensor(&sensor[SENSOR_G3D]);
    };

    int GetINA231(void) {
      int res;
    
      res =  read_sensor(&sensor[SENSOR_ARM]);
      res += read_sensor(&sensor[SENSOR_MEM]);
      res += read_sensor(&sensor[SENSOR_KFC]);
      res += read_sensor(&sensor[SENSOR_G3D]);
    
      armuV = (float)(sensor[SENSOR_ARM].data.cur_uV / 100000) / 10;
      armuA = (float)(sensor[SENSOR_ARM].data.cur_uA / 1000) / 1000;
      armuW = (float)(sensor[SENSOR_ARM].data.cur_uW / 1000) / 1000;
    
      memuV = (float)(sensor[SENSOR_MEM].data.cur_uV / 100000) / 10;
      memuA = (float)(sensor[SENSOR_MEM].data.cur_uA / 1000) / 1000;
      memuW = (float)(sensor[SENSOR_MEM].data.cur_uW / 1000) / 1000;
    
      kfcuV = (float)(sensor[SENSOR_KFC].data.cur_uV / 100000) / 10;
      kfcuA = (float)(sensor[SENSOR_KFC].data.cur_uA / 1000) / 1000;
      kfcuW = (float)(sensor[SENSOR_KFC].data.cur_uW / 1000) / 1000;
    
      g3duV = (float)(sensor[SENSOR_G3D].data.cur_uV / 100000) / 10;
      g3duA = (float)(sensor[SENSOR_G3D].data.cur_uA / 1000) / 1000;
      g3duW = (float)(sensor[SENSOR_G3D].data.cur_uW / 1000) / 1000;
    
      return res;
    };

private:
    sensor_t sensor[SENSOR_MAX];
    int mOldUserCPU[8];
    int mOldSystemCPU[8];
    int mOldIdleCPU[8];

};


namespace PMLib
{
    template <int n_lines = 4, int max_freq = 10, bool pdu = false>
    class OdroidXU3 : public Device {
      public:
        OdroidXU3(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu, 
            [&] () {

            GetNode getNode;

            if (getNode.OpenINA231()) {
              cout << "Error opening sensor" << endl;
              exit(-1);
            }

            while ( is_running() ){ 
                auto t1 = system_clock::now();

                getNode.GetINA231();
                /* A15 CPU Block */
                sample[0] = getNode.armuW;
                /* A7 CPU Block */
                sample[1] = getNode.kfcuW;
                /* GPU Block */
                sample[2] = getNode.g3duW;
                /* Memory Block */
                sample[3] = getNode.memuW;

                yield( sample );

                auto t2 = system_clock::now();
                auto dms = duration_cast<microseconds>(t2 - t1);
		if( dms.count() < (int)(1e6/max_freq) )
                  this_thread::sleep_for(std::chrono::microseconds((int)(1e6/max_freq) - dms.count()));
            }

            getNode.CloseINA231();
        } ) {};   
    };

    static RegisterDevice< OdroidXU3<> > Reg_OdroidXU3("OdroidXU3");
}

#endif
