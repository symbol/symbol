# image name required as ARG
ARG BUILD_IMAGE='symbolplatform/symbol-server-build-base:ubuntu-gcc-13'
ARG RELEASE_BASE_IMAGE='symbolplatform/symbol-server-build-base:ubuntu-gcc-13'
ARG DEBIAN_FRONTEND=noninteractive

FROM ${BUILD_IMAGE} AS builder

WORKDIR /tmp/build
COPY . src/

ARG COMPILER_CONFIGURATION='gcc-latest'
ARG BUILD_CONFIGURATION='release-public'
RUN ARCH=$([ "$(uname -m)" = "x86_64" ] && echo "amd64" || echo "arm64")  \
	&& python3 /tmp/build/src/jenkins/catapult/runDockerBuildInnerBuild.py \
	--compiler-configuration=/tmp/build/src/jenkins/catapult/configurations/${ARCH}/${COMPILER_CONFIGURATION}.yaml \
	--build-configuration=/tmp/build/src/jenkins/catapult/configurations/${BUILD_CONFIGURATION}.yaml \
	--source-path=/tmp/build/src/client/catapult \
	--out-dir=/tmp/build/binaries

FROM ${RELEASE_BASE_IMAGE}
LABEL maintainer="Catapult Development Team"

ARG CATAPULT_HOME=/usr/catapult
COPY --chown=1000:1000 --from=builder /tmp/build/binaries ${CATAPULT_HOME}
WORKDIR ${CATAPULT_HOME}
ENV LD_LIBRARY_PATH="${CATAPULT_HOME}/lib:${CATAPULT_HOME}/deps"
ARG USER_NAME='ubuntu'
USER ${USER_NAME}
