void call(String environment) {
	final String ownerName = env.GIT_URL.tokenize('/')[2]
	logger.logInfo("Configuring respository pull for ${environment}")

	configureArtifactRepositoryUrl(environment, ownerName)
	configureArtifactRepositoryLogin(environment, ownerName)
}

void configureNpmUrl(String url) {
	env.NPM_URL = url
	runScript('npm config set registry=$NPM_URL')
}

void npmLogin(String userName, String password) {
	env.USERNAME_PASSWORD_ENCODING = "${userName}:${password}".bytes.encodeBase64().toString()
	runScript('set +x; npm config set _auth=$USERNAME_PASSWORD_ENCODING')
	runScript('npm config set always-auth=true')
}

void configurePipUrl(String url) {
	env.PIP_INDEX_URL = url
}

void configureArtifactRepositoryUrl(String environment, String gitOrgName) {
	Map<String, Closure> handler = [
			'javascript' : this.&configureNpmUrl,
			'python': this.&configurePipUrl
	]

	helper.tryRunWithStringCredentials("${gitOrgName.toUpperCase()}_${environment.toUpperCase()}_ARTIFACTORY_URL_ID") { String url ->
		if (handler.containsKey(environment)) {
			handler[environment](url)
		}
	}
}

void configureArtifactRepositoryLogin(String environment, String gitOrgName) {
	Map<String, Closure> handler = [
			'javascript' : this.&npmLogin
	]

	helper.tryRunWithUserCredentials("${gitOrgName.toUpperCase()}_${environment.toUpperCase()}_ARTIFACTORY_LOGIN_ID") { String userName,
																														String password ->
		if (handler.containsKey(environment)) {
			handler[environment](userName, password)
		}
	}
}
