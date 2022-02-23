void call(Map config, String phase) {
	logger.logInfo("publishing data for ${phase}, ${config}")

	publisher(config, phase)
}

String readNpmPackageNameVersion() {
	Object packageJson = readJSON file: 'package.json'
	return "${packageJson.name}@${packageJson.version}"
}

String readPackageVersion() {
	return readFile(file: 'version.txt').trim()
}

Boolean isAlphaRelease(String phase) {
	return phase == 'alpha'
}

Boolean isRelease(String phase) {
	return phase == 'release'
}

void dockerPublisher(Map config, String phase) {
	if (config.publisher != 'docker' || config.dockerImageName == null) {
		return
	}

	final String dockerUrl = 'https://registry.hub.docker.com'
	String version = readPackageVersion()
	String imageVersionName = "${config.dockerImageName}:${version}"

	Object imageName = docker.build(imageVersionName)
	docker.withRegistry(dockerUrl, DOCKERHUB_CREDENTIALS_ID) {
		logger.logInfo("Pushing docker image ${imageVersionName}")
		imageName.push()
		if (isRelease(phase)) {
			logger.logInfo('Releasing the latest image')
			imageName.push('latest')
		}
	}
}

void npmPublisher(Map config, String phase) {
	if (config.publisher != 'npm') {
		return
	}

	String npmPublishCommand = 'npm publish'
	if (isAlphaRelease(phase)) {
		npmPublishCommand += ' --tag alpha'
	}

	// groovylint-disable-next-line GStringExpressionWithinString
	writeFile(file: '.npmrc', text: '//registry.npmjs.org/:_authToken=${NPM_TOKEN}')
	runScript('cat .npmrc')
	withCredentials([string(credentialsId: NPM_CREDENTIALS_ID, variable: 'NPM_TOKEN')]) {
		logger.logInfo("Publishing npm package ${readNpmPackageNameVersion()}")
		runScript(npmPublishCommand)
	}
}

void pythonPublisher(Map config, String phase) {
	if (config.publisher != 'pypi') {
		return
	}

	credentialsId = PYTHON_CREDENTIALS_ID
	if (isAlphaRelease(phase)) {
		credentialsId = TEST_PYTHON_CREDENTIALS_ID
	}

	Object requirementsFile = readFile 'requirements.txt'
	requirementsFile.readLines().each { line ->
		runScript("poetry add ${line}")
	}
	runScript('cat pyproject.toml')

	runScript('poetry build')

	withCredentials([string(credentialsId: credentialsId, variable: 'POETRY_PYPI_TOKEN_PYPI')]) {
		if (isAlphaRelease(phase)) {
			runScript('poetry config "repositories.test" "https://test.pypi.org/legacy/"')
			runScript("poetry config 'pypi-token.test' ${POETRY_PYPI_TOKEN_PYPI}")
			runScript('poetry publish --repository "test"')
		} else {
			runScript("poetry config 'pypi-token.pypi' ${POETRY_PYPI_TOKEN_PYPI}")
			runScript('poetry publish')
		}
	}
}

void gitHubPagesPublisher(Map config, String phase) {
	if (config.publisher != 'gh-pages' || !isRelease(phase)) {
		return
	}

	withCredentials([usernamePassword(credentialsId: config.gitHubId,
		usernameVariable: 'GITHUB_APP',
		passwordVariable: 'GITHUB_ACCESS_TOKEN')]) {
		helper.configureGitHub()

		withCredentials([usernamePassword(credentialsId: 'TRANSIFEX_LOGIN_ID',
			usernameVariable: 'TRANSIFEX_USER',
			passwordVariable: 'TRANSIFEX_PASSWORD')]) {
			runScript(env.GITHUB_PAGES_PUBLISH_SCRIPT_FILEPATH)
		}
	}
}

void publisher(Map config, String phase) {
	if (!config.publisher) {
		logger.logInfo('No publisher is configured.')
		return
	}

	if (!isAlphaRelease(phase) && !isRelease(phase)) {
		logger.logWarning('Publish phase should be alpha or release.')
		return
	}

	Closure[] strategies = [
		this.&dockerPublisher,
		this.&npmPublisher,
		this.&pythonPublisher,
		this.&gitHubPagesPublisher
	]

	strategies.each { publisher ->
		publisher.call(config, phase)
	}
}
