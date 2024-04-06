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

	final String ownerName = helper.resolveOrganizationName()
	final String repositoryName = helper.resolveRepositoryName()
	String dockerHost = 'registry.hub.docker.com'
	String dockerCredentialsId = DOCKER_CREDENTIALS_ID
	if (isAlphaRelease(phase) || !githubHelper.isGitHubRepositoryPublic(ownerName, repositoryName)) {
		dockerCredentialsId = "${ownerName.toUpperCase()}_ARTIFACTORY_LOGIN_ID"
		dockerHost = helper.resolveUrlHostName(configureArtifactRepository.resolveRepositoryUrl(ownerName, 'docker-hosted'))
	}

	final String version = readPackageVersion()
	final String imageVersionName = "${dockerHost}/${config.dockerImageName}:${version}"
	final String archImageName = imageVersionName + "-${ARCHITECTURE}"
	dockerHelper.loginAndRunCommand(dockerCredentialsId, dockerHost) {
		publishArtifact {
			String args = config.dockerBuildArgs ?: '.'
			args = '--network host ' + args
			dockerHelper.dockerBuildAndPushImage(archImageName, args)
			dockerHelper.updateDockerImage(imageVersionName, archImageName, "${ARCHITECTURE}")
			if (isRelease(phase)) {
				final String imageLatestName = "${dockerHost}/${config.dockerImageName}:latest"

				logger.logInfo('Releasing the latest image')
				dockerHelper.updateDockerImage(imageLatestName, archImageName, "${ARCHITECTURE}")
			}
		}
	}
}

void npmPublisher(Map config, String phase) {
	if (config.publisher != 'npm') {
		return
	}

	StringBuilder npmPublishCommand = new StringBuilder('npm publish')
	if (isAlphaRelease(phase)) {
		npmPublishCommand.append(' --tag alpha')
	}

	final String ownerName = helper.resolveOrganizationName()
	final String repositoryName = helper.resolveRepositoryName()
	if (isAlphaRelease(phase) || !githubHelper.isGitHubRepositoryPublic(ownerName, repositoryName)) {
		final String publishUrl = configureArtifactRepository.resolveRepositoryUrl(ownerName, 'npm-hosted')
		final String environment = jobHelper.resolveCiEnvironmentName(config)

		npmPublishCommand.append(" --registry=${publishUrl}")
		configureArtifactRepository.configure(environment, ownerName, publishUrl)
		publishArtifact {
			logger.logInfo("Publishing npm package ${readNpmPackageNameVersion()} to private repository")
			runScript(npmPublishCommand.toString())
		}
	} else {
		// groovylint-disable-next-line GStringExpressionWithinString
		writeFile(file: '.npmrc', text: '//registry.npmjs.org/:_authToken=${NPM_TOKEN}')
		runScript('cat .npmrc')
		withCredentials([string(credentialsId: NPM_CREDENTIALS_ID, variable: 'NPM_TOKEN')]) {
			publishArtifact {
				logger.logInfo("Publishing npm package ${readNpmPackageNameVersion()}")
				runScript(npmPublishCommand.toString())
			}
		}
	}
}

void pythonPublisher(Map config, String phase) {
	if (config.publisher != 'pypi') {
		return
	}

	final String ownerName = helper.resolveOrganizationName()
	final String repositoryName = helper.resolveRepositoryName()
	if (isAlphaRelease(phase) || !githubHelper.isGitHubRepositoryPublic(ownerName, repositoryName)) {
		withCredentials([usernamePassword(credentialsId: "${ownerName.toUpperCase()}_ARTIFACTORY_LOGIN_ID",
			usernameVariable: 'USERNAME',
			passwordVariable: 'PASSWORD')]) {
			publishArtifact {
				String publishUrl = configureArtifactRepository.resolveRepositoryUrl(ownerName, 'pypi-hosted')

				poetryBuildPackage()
				runScript("poetry config repositories.internal ${publishUrl}")
				runScript('poetry config http-basic.internal $USERNAME $PASSWORD')
				runScript('poetry publish --repository internal')
			}
		}
	} else {
		withCredentials([string(credentialsId: PYTHON_CREDENTIALS_ID, variable: 'PYPI_TOKEN')]) {
			publishArtifact {
				poetryBuildPackage()
				runScript('poetry config pypi-token.pypi $PYPI_TOKEN')
				runScript('poetry publish')
			}
		}
	}
}

void poetryBuildPackage() {
	Object requirementsFile = readFile 'requirements.txt'
	requirementsFile.readLines().each { line ->
		runScript("poetry add ${line}")
	}
	runScript('cat pyproject.toml')

	runScript('poetry build')
}

void gitHubPagesPublisher(Map config, String phase) {
	if (config.publisher != 'gh-pages' || !isRelease(phase)) {
		return
	}

	githubHelper.executeGitAuthenticatedCommand {
		withCredentials([usernamePassword(credentialsId: 'TRANSIFEX_LOGIN_ID',
			usernameVariable: 'TRANSIFEX_USER',
			passwordVariable: 'TRANSIFEX_PASSWORD')]) {
			runScript(env.GITHUB_PAGES_PUBLISH_SCRIPT_FILEPATH)
		}
	}
}

void awsPublisher(Map config, String phase) {
	if (config.publisher != 'aws' || !isRelease(phase)) {
		return
	}

	withCredentials([usernamePassword(credentialsId: config.awsCredentialId,
		usernameVariable: 'AWS_ACCESS_KEY_ID',
		passwordVariable: 'AWS_SECRET_ACCESS_KEY')]) {
		publishArtifact { }
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
		this.&gitHubPagesPublisher,
		this.&awsPublisher
	]

	strategies.each { publisher ->
		publisher.call(config, phase)
	}
}

void publishArtifact(Closure defaultPublisher) {
	final String publishScriptFilePath = 'scripts/ci/publish.sh'
	final String architecture = helper.resolveBuildArchitecture()

	if (fileExists(publishScriptFilePath)) {
		runScript("${publishScriptFilePath} ${architecture}")
	} else {
		defaultPublisher.call()
	}
}
