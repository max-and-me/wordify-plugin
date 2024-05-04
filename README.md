# VST 3 GPT Plug-In
[![CMake (Linux, macOS, Windows)](https://github.com/rehans/vst-gpt/actions/workflows/cmake.yml/badge.svg)](https://github.com/rehans/vst-gpt/actions/workflows/cmake.yml)

## Getting Started

To clone and create the project, open a command prompt and proceed as follows:

### Windows

```sh
git clone https://github.com/max-and-me/wordify-plugin.git
mkdir build
cd build
cmake ../wordify-plugin
cmake --build .
```

### macOS

```sh
git clone git clone https://github.com/max-and-me/wordify-plugin.git
mkdir build
cd build
cmake -GXcode ../wordify-plugin
cmake --build .
```

### Linux

```sh
git clone git clone https://github.com/max-and-me/wordify-plugin.git
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../wordify-plugin
cmake --build .
```

As soon as the project has been successfully built, you will find the plugin bundle in the build folder: ```Debug/VST3/Wordify.vst3```

### Installer

Build the project in ```Release``` configuration:

CPack is configured to work with the following Generators:

* Windows: ```INNOSETUP```
* macOS: ```productbuild```
* Linux: ```TGZ```

Execute CPack inside the CMake binary directory the ```project```. 

```sh
cpack -C Release -G <CPack_Generator> .
```

## Dependency Graph

![Alt text](doc/VstGPT.dot.VstGPT.png "Dependency Graph")

```shell
cmake --build . --target VstGPT-dependency-graph
cd graphviz
dot -Tpng -O ./VstGPT.dot.VstGPT
cp ./VstGPT.dot.VstGPT.png ../../vst-gpt/doc    
```

Copy the generated ```VstGPT.dot.VstGPT.png``` into the ```doc``` folder.

> TODO: Automate this!!

## How to whisper

How to use the whisper library:

```sh
git clone https://github.com/ggerganov/whisper.cpp.git
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ../whisper.cpp
cmake --build . --parallel
../whisper.cpp/models/download-ggml-model.sh base.en
cd bin
./main -m ../../whisper.cpp/models/ggml-base.en.bin -f ../../whisper.cpp/samples/jfk.wav -ml 1
```

> See also: https://github.com/ggerganov/whisper.cpp