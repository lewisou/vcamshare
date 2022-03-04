SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

build_android() {
    cd $SCRIPT_DIR/../depts/ffmpeg-kit/

    ./android.sh --lts --speed --no-archive
}