import java.nio.file.Paths

void call(Map config, String phase) {
	logger.logInfo("publishing data for ${phase}, ${config}")

	publisher(config, phase)
}

String getResource(String fileName) {
	return libraryResource(fileName)
}

void copyFileListToTemp(String[] files) {
	files.each { filepath ->
		fileHelper.copyToTempFile(getResource(Paths.get('artifacts').resolve(filepath).toString()), filepath)
	}
}

void copyWithJenkinsFunctionFileToTemp(String[] files) {
	copyFileListToTemp(files + ['scripts/jenkins-functions.sh'] as String[])
}

void dockerPublisher(Map config, String phase) {
	if (config.publisher == 'docker' && config.dockerImageName) {
		env.DOCKER_IMAGE_NAME = config.dockerImageName
		String[] files = ['scripts/docker-functions.sh']
		copyWithJenkinsFunctionFileToTemp(files)
		String scriptFullFilepath = fileHelper.copyToTempFile(
				getResource('artifacts/scripts/docker-publish.sh'),
				'scripts/docker-publish.sh')
		withCredentials([
			usernamePassword(
				credentialsId: DOCKERHUB_CREDENTIALS_ID,
				usernameVariable: 'DOCKER_USERNAME',
				passwordVariable: 'DOCKER_PASSWORD')
		]) {
			runScript("bash ${scriptFullFilepath} ${phase}")
		}
	}
}

void npmPublisher(Map config, String phase) {
	if (config.publisher == 'npm') {
		String[] files = ['scripts/node-functions.sh']
		copyWithJenkinsFunctionFileToTemp(files)
		String scriptFullFilepath = fileHelper.copyToTempFile(getResource('artifacts/scripts/node-publish.sh'), 'scripts/node-publish.sh')
		fileHelper.copyToLocalFile(getResource('artifacts/configuration/npmrc'), "${HOME}/.npmrc")
		withCredentials([string(credentialsId: NPM_CREDENTIALS_ID, variable: 'NPM_TOKEN')]) {
			runScript("bash ${scriptFullFilepath} ${phase}")
		}
	}
}

void pythonPublisher(Map config, String phase) {
	if (config.publisher != 'pypi') {
		return
	}

	credentialsId = PYTHON_CREDENTIALS_ID
	if (phase == 'alpha') {
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
