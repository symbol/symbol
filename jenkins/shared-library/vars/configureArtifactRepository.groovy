void call(String environment) {
	final String ownerName = env.GIT_URL.tokenize('/')[2]
	logger.logInfo("Configuring respository pull for ${environment}")

	configureArtifactRepository(environment, ownerName)
}

String readNpmPackageScopeName() {
	Object packageJson = readJSON file: 'package.json'
	String[] nameParts = packageJson.name.tokenize('/')
	return nameParts.length > 1 ? nameParts[0] : ''
}

void configureNpm(Map info) {
	final String scopeName = readNpmPackageScopeName()
	final String registryUrl = "registry=${info.url}"

	env.NPM_REGISTRY_URL = scopeName ? "${scopeName}:${registryUrl}" : registryUrl
	runScript('set +x && npm config set $NPM_REGISTRY_URL')
	if (!scopeName) {
		runScript('npm config set registry=https://registry.npmjs.org/')
	}

	if (info.userName != null && info.password != null) {
		String hostNamePath = info.url.split('://')[1]
		String userNamePasswordEncoding = "${info.userName}:${info.password}".bytes.encodeBase64()

		env.NPM_AUTH_ENCODING = "//${hostNamePath}:_auth=$userNamePasswordEncoding"
		runScript('set +x && npm config set $NPM_AUTH_ENCODING')
	}
}

void configurePip(Map info) {
	env.PIP_INDEX_URL = info.url
}

void configureArtifactRepository(String environment, String gitOrgName) {
	Map artifactRepositoryInfo = [:]
	Map<String, Closure> handler = [
			'javascript' : this.&configureNpm,
			'python': this.&configurePip
	]

	if (handler.containsKey(environment)) {
		final String artifactoryUrlId = "${gitOrgName.toUpperCase()}_${environment.toUpperCase()}_ARTIFACTORY_URL_ID"
		final String artifactoryLoginId = "${gitOrgName.toUpperCase()}_${environment.toUpperCase()}_ARTIFACTORY_LOGIN_ID"

		helper.tryRunWithStringCredentials(artifactoryUrlId) { String url ->
			artifactRepositoryInfo.url = url
		}

		helper.tryRunWithUserCredentials(artifactoryLoginId) { String userName, String password ->
			artifactRepositoryInfo.userName = userName
			artifactRepositoryInfo.password = password
		}

		handler[environment](artifactRepositoryInfo)
	}
}
