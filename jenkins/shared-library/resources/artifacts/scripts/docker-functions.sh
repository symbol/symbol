#!/usr/bin/env bash
set -x

SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
# shellcheck source=./shared-library/resources/artifacts/scripts/jenkins-functions.sh
. "$SCRIPTS_DIR/jenkins-functions.sh"

docker_build() {
	VERSION="$1"
	OPERATION="$2"

	validate_env_variable "DOCKER_IMAGE_NAME"
	validate_env_variable "VERSION"

	DOCKER_IMAGE_VERSION_NAME="${DOCKER_IMAGE_NAME}:${VERSION}"
	echo "Creating image ${DOCKER_IMAGE_VERSION_NAME}"
	docker build -t "${DOCKER_IMAGE_VERSION_NAME}" .

	if [[ "$OPERATION" == "alpha" || "$OPERATION" == "release" ]]; then
		echo "Building for operation ${OPERATION}..."
		validate_env_variable "DOCKER_USERNAME"
		validate_env_variable "DOCKER_PASSWORD"
		echo "Login into docker..."
		echo "${DOCKER_PASSWORD}" | docker login -u "${DOCKER_USERNAME}" --password-stdin

		if [ "$OPERATION" = "alpha" ]; then
			echo "Docker pushing alpha ${DOCKER_IMAGE_VERSION_NAME}"
			docker push "${DOCKER_IMAGE_VERSION_NAME}"
		fi

		if [ "$OPERATION" = "release" ]; then
			docker tag "${DOCKER_IMAGE_VERSION_NAME}" "${DOCKER_IMAGE_NAME}:latest"
			echo "Docker pushing release"
			docker push "${DOCKER_IMAGE_NAME}:latest"
			docker push "${DOCKER_IMAGE_VERSION_NAME}"
		fi

		DIGESTS=$(docker inspect --format='{{index .RepoDigests 0}}' "${DOCKER_IMAGE_VERSION_NAME}")
		echo "The DIGESTS is ${DIGESTS}"
	fi
}

if [ "$1" == "docker_build" ]; then
	docker_build "$2" "$3"
fi

if [ "$1" == "docker_build_version_file" ]; then
	docker_build "$(load_version_from_file)" "$2"
fi

