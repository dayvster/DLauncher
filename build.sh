APP_NAME="MyQtApp"
BUILD_DIR="build"

mkdir -p "$BUILD_DIR"

build_and_run() {
  cmake -B "$BUILD_DIR" -S .
  cmake --build "$BUILD_DIR"
  echo "--------------------------------"
  echo "🚀 Running $APP_NAME..."
  echo "--------------------------------"
  "./$BUILD_DIR/$APP_NAME"
  APP_PID=$!
}

build_and_run
