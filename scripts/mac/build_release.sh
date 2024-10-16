#!/bin/sh

# Display help function
function display_help {
    echo "Usage: $0 [option] arg1 arg2"
    echo
    echo "This script clones the repository, creates and builds the project, signs the binary, creates and signs the installer and runs Apples notary services."
    echo
    echo "Options:"
    echo "  -h, --help      Show this help message"
    echo
    echo "Arguments:"
    echo "  arg1            The first argument, Developer ID Application as string"
    echo "  arg2            The second argument, Developer ID Installer as string"
    echo
    echo "Example:"
    echo "  $0 'Developer ID Application: My Company (E53ZE5AABB)' 'Developer ID Installer: My Company (E53ZE5AABB)'"
    echo
    exit 0
}

# Check if -h or --help is passed
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    display_help
fi

# If no arguments are provided, show error and help
if [[ $# -lt 2 ]]; then
    echo "[MAM] MacOS_Build_Release_sh: Error, insufficient arguments provided."
    display_help
fi

# Cmake project
echo "[MAM] MacOS_Build_Release_sh: Clone Project"
git clone https://github.com/max-and-me/wordify-plugin.git
mkdir build
d build
echo "[MAM] MacOS_Build_Release_sh: CMake project"
cmake -GXcode ../wordify-plugin
cmake --build . --target Wordify --config Release

# Strip leading/trailing double quotes from arguments
arg1=$(echo "$1" | sed 's/^"//;s/"$//')
arg2=$(echo "$2" | sed 's/^"//;s/"$//')

# Sign the binaries
echo "[MAM] MacOS_Build_Release_sh: Sign the binaries"
codesign --sign "$1" -f -o runtime --timestamp -v ./VST3/Release/Wordify.vst3/Contents/MacOS/WordifyWorker
codesign --sign "$1" -f -o runtime --timestamp -v ./VST3/Release/Wordify.vst3/Contents/MacOS/Wordify
codesign --sign "$1" -f -o runtime --timestamp -v ./VST3/Release/Wordify.vst3

# Check binary signing
echo "[MAM] MacOS_Build_Release_sh: Check binary signing:"
codesign --display -vv ./VST3/Release/Wordify.vst3  

# Run the packager
echo "MacOS_Build_Release_sh: Run the packager:"
cpack -C Release -G productbuild .

# Sign the Installer
echo "[MAM] MacOS_Build_Release_sh: Sign the Installer:"
productsign --sign "$2" ./Wordify-2024.10-Darwin.pkg ./Wordify-2024.10-Darwin-signed.pkg  

# Check installer signing
echo "[MAM] MacOS_Build_Release_sh: Check installer signing:"
pkgutil --check-signature ./Wordify-2024.10-Darwin-signed.pkg

## Run the notary service
echo "[MAM] MacOS_Build_Release_sh: Run the notary service:"
xcrun notarytool submit ./Wordify-2024.10-Darwin-signed.pkg --keychain-profile NOTARYTOOL_PASSWORD --wait

## Run Stapler
echo "[MAM] MacOS_Build_Release_sh: Staple the result:"
xcrun stapler staple ./Wordify-2024.10-Darwin-signed.pkg 
