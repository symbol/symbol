import groovy.transform.Field

// Constants
@Field final Map PHASE = [
	ALPHA: 'alpha',
	RELEASE: 'release'
]

@Field final Map PUBLISHER_TYPE = [
	DOCKER: 'docker',
	NPM: 'npm',
	PYPI: 'pypi',
	GH_PAGES: 'gh-pages',
	AWS: 'aws',
	GRADLE: 'gradle'
]

@Field final Map REPOSITORY_TYPE = [
	DOCKER: 'docker-hosted',
	NPM: 'npm-hosted',
	PYPI: 'pypi-hosted',
	MAVEN: 'maven-releases'
]

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
	return phase == PHASE.ALPHA
}

Boolean isRelease(String phase) {
	return phase == PHASE.RELEASE
}

String resolveArtifactRepositoryName(String repositoryName, Boolean isPublicRepo) {
	return isPublicRepo ? repositoryName : "${repositoryName}-private"
}

Boolean shouldPublishToInternalRepository(String phase, Map config) {
	return isAlphaRelease(phase) || !config.isPublicGitHubRepo
}

String resolveArtifactoryCredentialsId() {
	final String ownerName = helper.resolveOrganizationName()
	return "${ownerName.toUpperCase()}_ARTIFACTORY_LOGIN_ID"
}

String resolveInternalRepositoryUrl(String repositoryType, Map config) {
	final String artifactRepositoryName = resolveArtifactRepositoryName(repositoryType, config.isPublicGitHubRepo)
	final String ownerName = helper.resolveOrganizationName()
	return configureArtifactRepository.resolveRepositoryUrl(ownerName, artifactRepositoryName)
}

void dockerPublisher(Map config, String phase) {
	if (config.publisher != PUBLISHER_TYPE.DOCKER || config.dockerImageName == null) {
		return
	}

	String dockerHost = 'registry.hub.docker.com'
	String dockerCredentialsId = DOCKER_CREDENTIALS_ID
	if (shouldPublishToInternalRepository(phase, config)) {
		dockerHost = helper.resolveUrlHostName(resolveInternalRepositoryUrl(REPOSITORY_TYPE.DOCKER, config))
		dockerCredentialsId = resolveArtifactoryCredentialsId()
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
	if (config.publisher != PUBLISHER_TYPE.NPM) {
		return
	}

	StringBuilder npmPublishCommand = new StringBuilder('npm publish')
	if (isAlphaRelease(phase)) {
		npmPublishCommand.append(' --tag alpha')
	}

	if (shouldPublishToInternalRepository(phase, config)) {
		final String publishUrl = resolveInternalRepositoryUrl(REPOSITORY_TYPE.NPM, config)
		final String environment = jobHelper.resolveCiEnvironmentName(config)

		npmPublishCommand.append(" --registry=${publishUrl}")
		final String ownerName = helper.resolveOrganizationName()
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
	if (config.publisher != PUBLISHER_TYPE.PYPI) {
		return
	}

	if (shouldPublishToInternalRepository(phase, config)) {
		withCredentials([usernamePassword(credentialsId: resolveArtifactoryCredentialsId(),
			usernameVariable: 'USERNAME',
			passwordVariable: 'PYPI_TOKEN')]) {
			String publishUrl = resolveInternalRepositoryUrl(REPOSITORY_TYPE.PYPI, config)
			env.PYPI_URL = publishUrl

			publishArtifact {
				poetryBuildPackage()
				runScript("poetry config repositories.internal ${publishUrl}")
				runScript('poetry config http-basic.internal $USERNAME $PYPI_TOKEN')
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
	if (config.publisher != PUBLISHER_TYPE.GH_PAGES || !isRelease(phase)) {
		return
	}

	githubHelper.executeGitAuthenticatedCommand {
		withCredentials([string(credentialsId: 'TRANSIFEX_TOKEN_ID', variable: 'TRANSIFEX_TOKEN')]) {
			runScript(env.GITHUB_PAGES_PUBLISH_SCRIPT_FILEPATH)
		}
	}
}

void awsPublisher(Map config, String phase) {
	if (config.publisher != PUBLISHER_TYPE.AWS || !isRelease(phase)) {
		return
	}

	withCredentials([usernamePassword(credentialsId: config.awsCredentialId,
		usernameVariable: 'AWS_ACCESS_KEY_ID',
		passwordVariable: 'AWS_SECRET_ACCESS_KEY')]) {
		publishArtifact { }
	}
}

void gradlePublisher(Map config, String phase) {
	if (config.publisher != PUBLISHER_TYPE.GRADLE) {
		return
	}

	String credentialsId = 'MAVEN_CREDENTIALS_ID'
	String gradleTask="publishCentral"
	String usernameEnvironmentName = 'ORG_GRADLE_PROJECT_mavenCentralUsername'
	String passwordEnvironmentName = 'ORG_GRADLE_PROJECT_mavenCentralPassword'
	if (shouldPublishToInternalRepository(phase, config)) {
		usernameEnvironmentName = 'ORG_GRADLE_PROJECT_mavenRepoUsername'
		passwordEnvironmentName = 'ORG_GRADLE_PROJECT_mavenRepoPassword'
		credentialsId = resolveArtifactoryCredentialsId()
		env.ORG_GRADLE_PROJECT_mavenRepoUrl = resolveInternalRepositoryUrl(REPOSITORY_TYPE.MAVEN, config)
		gradleTask="publishInternal"
	}

	withCredentials([usernamePassword(credentialsId: credentialsId,
		usernameVariable: usernameEnvironmentName,
		passwordVariable: passwordEnvironmentName)]) {
		publishArtifact {
			runScript("./gradlew ${gradleTask} --debug")
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
		this.&gitHubPagesPublisher,
		this.&awsPublisher,
		this.&gradlePublisher
	]

	strategies.each { publisherStrategy ->
		publisherStrategy.call(config, phase)
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
