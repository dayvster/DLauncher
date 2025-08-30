#!/usr/bin/env bash
set -e

APP_NAME="MyQtApp"
BUILD_DIR="build"

mkdir -p "$BUILD_DIR"

build_and_run() {
  cmake -B "$BUILD_DIR" -S .
  cmake --build "$BUILD_DIR"
  echo "--------------------------------"
  echo "🚀 Running $APP_NAME..."
  echo "--------------------------------"
  "./$BUILD_DIR/$APP_NAME" &
  APP_PID=$!
}

cleanup() {
  if [[ -n "$APP_PID" ]]; then
    kill "$APP_PID" 2>/dev/null || true
  fi
}
trap cleanup EXIT

build_and_run

echo "👀 Watching for changes..."
while inotifywait -qr -e modify,create,delete --exclude 'build/*' .; do
  cleanup
  build_and_run
done
