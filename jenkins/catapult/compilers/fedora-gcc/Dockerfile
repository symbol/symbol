ARG FROM_IMAGE=fedora:41
FROM ${FROM_IMAGE}

RUN dnf update --assumeyes >/dev/null && \
	dnf install --assumeyes \
	gnupg2 \
	gcc-c++ \
	curl \
	xz \
	wget \
	>/dev/null && \
	dnf clean all && \
	rm -rf /var/cache/yum

# add fedora user (used by jenkins)
ARG HOME_DIR=/home/fedora
RUN groupadd -g 1000 fedora && \
	useradd -m -u 1000 -g fedora -d ${HOME_DIR} fedora
USER fedora
WORKDIR ${HOME_DIR}
