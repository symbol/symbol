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

RUN id -u "fedora" || useradd --uid 1000 -ms /bin/bash fedora
USER fedora
WORKDIR /home/fedora
