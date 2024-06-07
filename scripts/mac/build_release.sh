 #!/usr/bin/env bash

 git clone https://github.com/max-and-me/wordify-plugin.git
 mkdir build
 cd build
 cmake -GXcode ../wordify-plugin
 cmake --build . --target Wordify --config Release
 cpack -C Release -G productbuild .