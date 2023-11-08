# PMLib
PMLib: Power Measurement Library

## Installation guide

1. Install dependencies

```bash
sudo apt-get install libboost-all-dev
```

2. Clone the repository 

```bash
git clone http://lorca.act.uji.es/gitlab/leon/pmlib.git
```

3. Access to server folder 

```bash
cd pmlib/server
```

4. Make a new folder called "build" and navigate to it

```bash
mkdir build
cd build
```

5. Compile

```bash
cmake ..
```

6. Make

```bash
make
```

7. Navigate to client folder and make

```bash
cd /pmlib/client
make
```

8. Modify the settings.json of your device to match the file path to the server folder

