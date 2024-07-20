#! /bin/sh

# Change directory to the script directory
cd "$(dirname "$0")"

# Define some vars here
COMPANY_NAME="WordifyOrg"
PRODUCT_NAME="Wordify"
LICENSE_FILE="LICENSE.txt"
SRCPATH=$(pwd)

# From the VST3 documentation
VST3DIR="$HOME/.vst3"
DATADIR="$HOME/$COMPANY_NAME"
mkdir -p ${VST3DIR}

# Help will be show when -h|--help
print_help()
{
    echo "USAGE:"
    echo "    ./install        [OPTIONS]"
    echo
    echo "FLAGS:"
    echo "    -h, --help       Prints help information"
    echo
    echo "OPTIONS:"
    echo "    -l, --license    Show license information and exit."
    echo "    -u, --uninstall  Call the uninstaller."
    exit
}

# Simply prints the license.txt into the terminal.
print_license()
{
    FILE=${SRCPATH}/${LICENSE_FILE}
    cat "${FILE}"
    exit
}

# Copies files into target directories.
install()
{
    echo "Installing..."
    echo
    SOURCE="${SRCPATH}/${PRODUCT_NAME}.vst3"
    TARGET="${VST3DIR}"
    echo "  Copy ${PRODUCT_NAME}.vst3 to ${TARGET} ... "
    if cp -R ${SOURCE} ${TARGET}
        then echo "  ...done!"
        else echo "  ...failed!"
    fi

    SOURCE="${SRCPATH}/${COMPANY_NAME}"
    TARGET="${DATADIR}"
    echo "  Copy ${PRODUCT_NAME} data to ${DATADIR} ... "
    if mkdir -p "${TARGET}" && cp -R "${SOURCE}/." "$_" # $_ is return of mkdir
        then echo "  ...done!"
        else echo "  ...failed!"
    fi

    # Only install if apt-get is available e.g. Ubuntu. On Manjaro, no need to install anything.
    if [ -x "$(command -v apk-get)" ];
        then 
        # $ Use 'ldd Wordify.so' to list all dependencies.
        echo "  Install system packages ... "
        sudo apt-get install libfreetype6 libx11-xcb1 \
        libfreetype6 \
        libxcb1 \
        libxcb-util1 \
        libxcb-cursor0 \
        libxcb-xkb1 \
        libxkbcommon0 \
        libxkbcommon-x11-0 \
        libcairo2 \
        libfontconfig1 \
        libexpat1 \
        libstdc++6 \
        libsqlite3-0 \
        libxcb-keysyms1
        echo "  ...done!" 
    fi
    
    exit
}

# Deletes folders from where they have been installed by this installer.
uninstall()
{
    DIR="${VST3DIR}/${PRODUCT_NAME}.vst3"
    rm -rf ${DIR}
    exit
}

# Parse script options.
while [ $# -gt 0 ] ;
do
    case "$1" in
        -h|--help)
            print_help
            ;;
        -l|--license)
            print_license
            ;;
        -u|--uninstall)
            uninstall
            ;;
        *) echo "Invalid options passed! Use --help to show valid options." ; exit 1 ;;
    esac
done

# Will be called if no options have been passed.
install