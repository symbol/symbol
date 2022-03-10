defaultCiPipeline {
	platform = ['ubuntu-xlarge']
	ciBuildDockerfile = 'cpp.Dockerfile'
	dockerArgs = '--env=CC=gcc \
		--env=CCACHE_DIR=/ccache \
		--env=CXX=g++ \
		--volume=/jenkins_cache/ccache/ci:/ccache:rw \
		--volume=/jenkins_cache/conan/gcc:/conan \
		--add-host=db:127.0.0.1 \
		--ip6 2001:db8:1::20'

	packageId = 'client-catapult'
}
