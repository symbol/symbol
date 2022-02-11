import java.nio.file.Paths

void call(Map config, String phase) {
	logger.logInfo("publishing data for ${phase}, ${config}")

	publisher(config, phase)
}

String getResource(String fileName) {
	return libraryResource(fileName)
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
		if (phase == 'release') {
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

	fileHelper.copyToLocalFile(getResource('artifacts/configuration/npmrc'), "${HOME}/.npmrc")
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

	String scriptFullFilepath = fileHelper.copyToTempFile(getResource('artifacts/scripts/pypi-publish.sh'), 'scripts/pypi-publish.sh')
	withCredentials([string(credentialsId: credentialsId, variable: 'POETRY_PYPI_TOKEN_PYPI')]) {
		runScript("bash ${scriptFullFilepath} ${phase}")
	}
}

void publisher(Map config, String phase) {
	if (!config.publisher) {
		logger.logInfo('No publisher is configured.')
		return
	}

	Closure[] strategies = [
		this.&dockerPublisher,
		this.&npmPublisher,
		this.&pythonPublisher
	]

	strategies.each { publisher ->
		publisher.call(config, phase)
	}
}
