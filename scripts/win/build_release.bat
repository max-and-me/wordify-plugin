git clone https://github.com/max-and-me/wordify-plugin.git
mkdir build
cd build
cmake ..\wordify-plugin
cmake --build . --config Release
cpack -C Release -G INNOSETUP .