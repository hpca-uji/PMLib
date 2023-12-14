# PMLib
PMLib: Power Measurement Library

## Installation guide

### 1. Install dependencies

```bash
sudo apt-get install libboost-all-dev
```

### 2. Clone the repository 

```bash
git clone http://lorca.act.uji.es/gitlab/leon/pmlib.git
```

### 3. Access to server folder 

```bash
cd pmlib/server
```

### 4. Make a new folder called "build" and navigate to it

```bash
mkdir build
cd build
```

### 5. Compile

```bash
cmake ..
```

### 6. Make

```bash
make
```

### 7. Navigate to client folder and make

```bash
cd pmlib/client
make
```

### 8. Modify the "log_filename" in the settings-device.json of your device to match the file path to the server folder. As an example, here is the structure for the Jetson Nano in the settings-nano.json. 

```json
{
        "server_ip"    : "127.0.0.1",
        "server_port"  : 6526,
        "log_filename" : "/pmlib/server",  

    "computers" : {
        "Jetson-AGX2" :    { "ip" : "0.0.0.0" }
        },

    "devices" : {
        "Jetson-Nano" : { "type" : "Nano", "url": "/sys/bus/i2c/drivers/ina3221x/", "max_freq" : 10,
            "lines" : {
                "POM_5V_IN"     : { "number" : 0,  "voltage": 19, "description" : "POM_5V_IN" },
                "POM_5V_GPU"    : { "number" : 1,  "voltage": 19, "description" : "POM_5V_GPU" },
                "POM_5V_CPU"    : { "number" : 2,  "voltage": 19, "description" : "POM_5V_CPU" }
                }
            }
        }
    }
}
```

## Example usage

### 1. Set the server up in a terminal with the settings file of your device

```bash
cd pmlib/server/build
sudo ./pmlib_server --configfile ../settings-nano.json
```

### 2. In another terminal navigate to the client and test the library

```bash
cd pmlib/client
./pm_info -s 127.0.0.1:6526 -l -r Jetson-Nano
```

Data should be displayed in a 10 Hz timing ratio

### 3. Execute the pmlib_example.py, where the Power Consumed will be measured for 2 seconds of the device

```bash
python3 pmlib_example.py
```



