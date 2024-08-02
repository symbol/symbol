ARG FROM_IMAGE='ubuntu:24.04'

FROM ${FROM_IMAGE}

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl zip unzip

# install golang
RUN ARCH=$([ "$(uname -m)" = "x86_64" ] && echo "amd64" || echo "arm64") \
	&& curl -O -L "https://go.dev/dl/go1.22.5.linux-${ARCH}.tar.gz" \
	&& rm -rf /usr/local/go \
	&& tar -C /usr/local -xzf go1.22.5.linux-${ARCH}.tar.gz
ENV PATH=$PATH:/usr/local/go/bin

# codecov uploader
RUN ARCH=$([ "$(uname -m)" = "x86_64" ] && echo "linux" || echo "aarch64") \
	&& curl -Os "https://uploader.codecov.io/latest/${ARCH}/codecov" \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu
USER ubuntu
WORKDIR /home/ubuntu
ENV HOME=/home/ubuntu
ENV GOPATH=$HOME/go
ENV PATH=$PATH:$GOPATH/bin

# binary will be $(go env GOPATH)/bin/golangci-lint
RUN curl -sSfL https://raw.githubusercontent.com/golangci/golangci-lint/master/install.sh | sh -s -- -b $(go env GOPATH)/bin v1.59.1 \
	&& golangci-lint --version
