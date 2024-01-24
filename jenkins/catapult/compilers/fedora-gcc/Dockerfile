ARG FROM_IMAGE=fedora:39
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
