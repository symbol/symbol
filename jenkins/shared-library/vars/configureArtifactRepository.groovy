void call(String environment) {
	final String ownerName = helper.resolveOrganizationName()
	logger.logInfo("Configuring respository pull for ${environment}")

	String url = resolveRepositoryUrl(ownerName, resolveRepositoryName(environment))

	if (null != url) {
		configure(environment, ownerName, url)
	}
}

String resolveRepositoryName(String environment) {
	Map<String, List<Object>> environmentRepoNameMap = [
		'javascript' : 'npm-group',
		'python': 'pypi-group'
	]

	return environmentRepoNameMap[environment] ?: ''
}

String readNpmPackageScopeName() {
	Object packageJson = readJSON file: 'package.json'
	String[] nameParts = packageJson.name.tokenize('/')
	return nameParts.length > 1 ? nameParts[0] : null
}

void configureNpm(Map info) {
	final String scopeName = readNpmPackageScopeName()
	String registryUrl = "registry=${info.url}"
	String npmrcContent = scopeName ? "${scopeName}:${registryUrl}" : registryUrl

	if (null != scopeName) {
		npmrcContent += '\nregistry=https://registry.npmjs.org/'
	}

	if (null != info.userName && null != info.password) {
		final String hostNamePath = info.url.split('://')[1]
		final String userNamePasswordEncoding = "${info.userName}:${info.password}".bytes.encodeBase64()

		npmrcContent += "\n//${hostNamePath}:_auth=${userNamePasswordEncoding}"
	}

	writeFile(file: '.npmrc', text: npmrcContent)
}

void configurePypi(Map info) {
	final String hostName = helper.resolveUrlHostName(info.url)

	env.PIP_INDEX_URL = "${info.url}simple"
	env.PIP_TRUSTED_HOST = "${hostName} pypi.org"
	env.PIP_EXTRA_INDEX_URL = 'https://pypi.org/simple'
	if (null != info.userName && null != info.password) {
		env.NETRC = pwd()
		// groovylint-disable-next-line GStringExpressionWithinString
		writeFile(file: '.netrc', text: """
			machine ${hostName}
			login ${info.userName}
			password ${info.password}
		""")
	}
}

String resolveRepositoryUrl(String ownerName, String repositoryName) {
	final String artifactoryUrlId = "${ownerName.toUpperCase()}_ARTIFACTORY_URL_ID"
	String repositoryUrl = null

	helper.tryRunCommand {
		withCredentials([string(credentialsId: artifactoryUrlId, variable: 'ARTIFACTORY_URL')]) {
			repositoryUrl = env.ARTIFACTORY_URL.toString().endsWith('/') ? env.ARTIFACTORY_URL : "${env.ARTIFACTORY_URL}/"
			repositoryUrl += "${repositoryName}/"
		}
	}

	return repositoryUrl
}

List<String> resolveRepositoryUserNamePassword(String ownerName) {
	final String artifactoryLoginId = "${ownerName.toUpperCase()}_ARTIFACTORY_LOGIN_ID"
	List<String> userNamePassword = null

	helper.tryRunCommand {
		withCredentials([usernamePassword(credentialsId: artifactoryLoginId,
				usernameVariable: 'USERNAME',
				passwordVariable: 'PASSWORD')]) {
			userNamePassword = [env.USERNAME, env.PASSWORD]
		}
	}

	return userNamePassword
}

void configure(String environment, String gitOrgName, String url) {
	Map artifactRepositoryInfo = [:]
	Map<String, List<Object>> repositoryHandlerMap = [
		'javascript' : this.&configureNpm,
		'python': this.&configurePypi
	]

	if (repositoryHandlerMap.containsKey(environment)) {
		artifactRepositoryInfo.url = url
		List<String> userNamePassword = resolveRepositoryUserNamePassword(gitOrgName)
		if (null != userNamePassword) {
			artifactRepositoryInfo.userName = userNamePassword[0]
			artifactRepositoryInfo.password = userNamePassword[1]
		}

		repositoryHandlerMap[environment](artifactRepositoryInfo)
	}
}
