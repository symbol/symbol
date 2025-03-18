FROM ubuntu:24.04

# install dependencies (install tzdata first to prevent 'geographic area' prompt)
RUN apt-get update \
	&& apt-get install -y tzdata \
	&& apt-get install -y openjdk-11-jdk-headless git curl libssl-dev maven ca-certificates zip unzip \
	&& update-ca-certificates

# install python
RUN apt-get install -y python3-pip python3-venv

# install shellcheck
RUN apt-get install -y shellcheck

# nodejs
ENV NODE_OPTIONS="--dns-result-order=ipv4first"
ARG NODEJS_VERSION=22
RUN apt-get install -y ca-certificates curl gnupg \
	&& mkdir -p /etc/apt/keyrings \
	&& curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
	&& echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_${NODEJS_VERSION}.x nodistro main" \
	| tee /etc/apt/sources.list.d/nodesource.list \
	&& apt-get update \
	&& apt-get install -y nodejs

RUN npm install -g typedoc typedoc-plugin-markdown

# doxygen
RUN apt-get install -y bison cmake flex gcc g++ graphviz make qtcreator qt6-tools-dev texlive-full
ARG DOXYGEN_VERSION=1.13.2
RUN curl -Os https://www.doxygen.nl/files/doxygen-${DOXYGEN_VERSION}.src.tar.gz \
	&& tar -xf doxygen-${DOXYGEN_VERSION}.src.tar.gz \
	&& cd doxygen-${DOXYGEN_VERSION} \
	&& mkdir build \
	&& cd build \
	&& cmake -G "Unix Makefiles" .. \
	&& cmake -Dbuild_wizard=YES .. \
	&& make \
	&& make install \
	&& cd ../.. \
	&& rm -rf doxygen-${DOXYGEN_VERSION} doxygen-${DOXYGEN_VERSION}.src.tar.gz

# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu
USER ubuntu
WORKDIR /home/ubuntu
ENV PATH=$PATH:/home/ubuntu/.local/bin

# create a virtual environment
ENV VIRTUAL_ENV=/home/ubuntu/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# install gitlint
RUN pip install gitlint

# install gradle
SHELL ["/bin/bash", "-c"]
RUN curl -s "https://get.sdkman.io" | bash \
	&& source "/home/ubuntu/.sdkman/bin/sdkman-init.sh" \
	&& sdk install gradle
ENV PATH="$PATH:/home/ubuntu/.sdkman/candidates/gradle/current/bin"
WORKDIR /home/ubuntu
