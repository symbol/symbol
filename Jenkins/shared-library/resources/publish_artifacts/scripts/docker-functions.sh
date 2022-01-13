#!/usr/bin/env bash
#set -e

echo "path is $(dirname $0)"
SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
. $SCRIPTS_DIR/Jenkins-functions.sh

docker_build(){
  VERSION="$1"
  OPERATION="$2"

  validate_env_variable "DOCKER_IMAGE_NAME" "$FUNCNAME"
  validate_env_variable "VERSION" "$FUNCNAME"

  DOCKER_IMAGE_VERSION_NAME="${DOCKER_IMAGE_NAME}:${VERSION}"
  echo "Creating image ${DOCKER_IMAGE_VERSION_NAME}"
  docker build -t "${DOCKER_IMAGE_VERSION_NAME}" .

  if [[ "$OPERATION" == "alpha" || "$OPERATION" == "release"  ]];
  then
      echo "Building for operation ${OPERATION}..."
      validate_env_variable "DOCKER_USERNAME" "$FUNCNAME"
      validate_env_variable "DOCKER_PASSWORD" "$FUNCNAME"
      echo "Login into docker..."
      echo "${DOCKER_PASSWORD}" | docker login -u "${DOCKER_USERNAME}" --password-stdin

      if [ "$OPERATION" = "alpha" ]
      then
          TIMESTAMP_IMAGE_VERSION_NAME="${DOCKER_IMAGE_VERSION_NAME}-$(date +%Y%m%d%H%M)"
          docker tag "${DOCKER_IMAGE_VERSION_NAME}" "${TIMESTAMP_IMAGE_VERSION_NAME}"
          echo "Docker pushing alpha ${TIMESTAMP_IMAGE_VERSION_NAME}"
          docker push "${TIMESTAMP_IMAGE_VERSION_NAME}"
      fi

      if [ "$OPERATION" = "release" ]
      then
          docker tag "${DOCKER_IMAGE_VERSION_NAME}" "${DOCKER_IMAGE_NAME}:latest"
          echo "Docker pushing release"
          docker push "${DOCKER_IMAGE_NAME}:latest"
          docker push "${DOCKER_IMAGE_VERSION_NAME}"
      fi
  fi
}

if [ "$1" == "docker_build" ];then
    docker_build $2 $3
fi

if [ "$1" == "docker_build_version_file" ];then
    docker_build "$(load_version_from_file)" $2
fi

