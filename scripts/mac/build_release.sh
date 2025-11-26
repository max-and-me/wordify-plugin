#!/bin/sh

# Exit immediately if any command fails
set -e  

# Display help function
function display_help {
    echo "Usage: $0 [option] arg1 arg2 arg3"
    echo
    echo "This script clones the repository, creates and builds the project, signs the binary, creates and signs the installer and runs Apples notary services."
    echo
    echo "Options:"
    echo "  -h, --help      Show this help message"
    echo
    echo "Arguments:"
    echo "  arg1            The first argument, Developer ID Application as string"
    echo "  arg2            The second argument, Developer ID Installer as string"
    echo "  arg3            The third argument, Git tag as string (Optional)"
    echo
    echo "Example:"
    echo "  $0 'Developer ID Application: My Company (E53ZE5AABB)' 'Developer ID Installer: My Company (E53ZE5AABB)' v2025.02"
    echo
    exit 0
}

# Check if -h or --help is passed
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    display_help
fi

# If no arguments are provided, show error and help
if [[ $# -le 2 ]]; then
    echo "[MAM] MacOS_Build_Release_sh: Error, insufficient arguments provided."
    display_help
fi

# Clone project
echo "[MAM] MacOS_Build_Release_sh: Clone Project"
git clone https://github.com/max-and-me/wordify-plugin.git

echo "[MAM] MacOS_Build_Release_sh: Checkout Project with tag" 
if [ -n "$3" ]; then
    cd wordify-plugin
    git checkout -b checkout-at-"$3" "$3"
    cd ..
fi

# Cmake project
mkdir build
cd build
echo "[MAM] MacOS_Build_Release_sh: CMake project"
cmake -GXcode -DSMTG_BUILD_UNIVERSAL_BINARY=OFF ../wordify-plugin
cmake --build . --target Wordify --config Release

# Sign the binaries
echo "[MAM] MacOS_Build_Release_sh: Sign the binaries"
codesign --sign "$1" -f -o runtime --timestamp -v ./VST3/Release/Wordify.vst3/Contents/MacOS/whisper-cli
codesign --sign "$1" -f -o runtime --timestamp -v ./VST3/Release/Wordify.vst3/Contents/MacOS/Wordify
codesign --sign "$1" -f -o runtime --timestamp -v ./VST3/Release/Wordify.vst3

# Check binary signing
echo "[MAM] MacOS_Build_Release_sh: Check binary signing:"
codesign --display -vv ./VST3/Release/Wordify.vst3  

# Run the packager
echo "MacOS_Build_Release_sh: Run the packager:"
cpack -C Release -G productbuild .

# Find the generated installer 
installer_name=$(ls *.pkg)

# Check if a .pkg file exists
if [ -z "$installer_name" ]; then
    echo "No .pkg file found"
    exit 1
fi

# Handle multiple .pkg files
if [ $(echo "$installer_name" | wc -l) -ne 1 ]; then
    echo "Multiple .pkg files found:"
    echo "$installer_name"
    echo "Please make sure only one package exists."
    exit 1
fi

# Remove extension 
installer_name_no_ext=${installer_name%.pkg}
installer_name_signed="${installer_name_no_ext}-signed.pkg"

# Sign the Installer
echo "[MAM] MacOS_Build_Release_sh: Sign the Installer:"
productsign --sign "$2" "./${installer_name}" "./${installer_name_signed}"  

# Check installer signing
echo "[MAM] MacOS_Build_Release_sh: Check installer signing:"
pkgutil --check-signature "./${installer_name_signed}" 

## Run the notary service
echo "[MAM] MacOS_Build_Release_sh: Run the notary service:"
xcrun notarytool submit "./${installer_name_signed}" --keychain-profile NOTARYTOOL_PASSWORD --wait

## Run Stapler
echo "[MAM] MacOS_Build_Release_sh: Staple the result:"
xcrun stapler staple "./${installer_name_signed}"
