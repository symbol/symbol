pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: "${env.GIT_BRANCH}", name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'COMPILER_CONFIGURATION',
			choices: ['gcc-latest', 'gcc-prior', 'gcc-westmere', 'clang-latest', 'clang-prior'],
			description: 'compiler configuration'
		choice name: 'BUILD_CONFIGURATION',
			choices: ['release-private', 'release-public'],
			description: 'build configuration'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'fedora'],
			description: 'operating system'
		choice name: 'ARCHITECTURE',
			choices: ['amd64', 'arm64'],
			description: 'Computer architecture'

		booleanParam name: 'SHOULD_PUBLISH_BUILD_IMAGE', description: 'true to publish build image', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName("${OPERATING_SYSTEM}", "${ARCHITECTURE}", 'xlarge')}"
	}

	environment {
		DOCKER_URL = 'https://registry.hub.docker.com'
		DOCKER_CREDENTIALS_ID = 'docker-hub-token-symbolserverbot'
	}

	options {
		ansiColor('css')
		timestamps()
	}

	stages {
		stage('prepare') {
			stages {
				stage('git checkout') {
					when {
						expression { isManualBuild() }
					}
					steps {
						dir('symbol-mono') {
							sh "git checkout ${resolveBranchName()}"
							sh "git reset --hard origin/${resolveBranchName()}"
						}
					}
				}
				stage('prepare variables') {
					steps {
						script {
							fullyQualifiedUser = sh(
								script: 'echo "$(id -u):$(id -g)"',
								returnStdout: true
							).trim()

							final String ownerName = helper.resolveOrganizationName()
							dockerCredentialId = helper.isPublicBuild(params.BUILD_CONFIGURATION)
									? env.DOCKER_CREDENTIALS_ID
									: "${ownerName.toUpperCase()}_ARTIFACTORY_LOGIN_ID"
							dockerUrl = helper.isPublicBuild(params.BUILD_CONFIGURATION)
									? env.DOCKER_URL
									: helper.resolveUrlBase(configureArtifactRepository.resolveRepositoryUrl(ownerName, 'docker-hosted'))

							compilerConfigurationFilepath = "symbol-mono/jenkins/catapult/configurations/${ARCHITECTURE}/${COMPILER_CONFIGURATION}.yaml"
							imageLabel = resolveImageLabel(compilerConfigurationFilepath)
							dockerRepoName = "symbolplatform/${resolveImageRepo()}"
							buildImageFullName = "${dockerRepoName}:${imageLabel}"
						}
					}
				}
				stage('print env') {
					steps {
						echo """
									env.GIT_BRANCH: ${env.GIT_BRANCH}
									env.GIT_COMMIT: ${env.GIT_COMMIT}
								 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}

							COMPILER_CONFIGURATION: ${COMPILER_CONFIGURATION}
							   BUILD_CONFIGURATION: ${BUILD_CONFIGURATION}
								  OPERATING_SYSTEM: ${OPERATING_SYSTEM}
									  ARCHITECTURE: ${ARCHITECTURE}

						SHOULD_PUBLISH_BUILD_IMAGE: ${SHOULD_PUBLISH_BUILD_IMAGE}

								fullyQualifiedUser: ${fullyQualifiedUser}
										imageLabel: ${imageLabel}
								buildImageFullName: ${buildImageFullName}
						"""
					}
				}
			}
		}
		stage('build') {
			stages {
				stage('prepare variables') {
					steps {
						script {
							runDockerBuildCommand = """
								python3 symbol-mono/jenkins/catapult/runDockerBuild.py \
									--compiler-configuration ${compilerConfigurationFilepath} \
									--build-configuration symbol-mono/jenkins/catapult/configurations/${BUILD_CONFIGURATION}.yaml \
									--operating-system ${OPERATING_SYSTEM} \
									--user ${fullyQualifiedUser} \
									--destination-image-label ${imageLabel} \
									--source-path symbol-mono \
							"""
						}
					}
				}
				stage('pull dependency images') {
					steps {
						script {
							baseImageNames = sh(
								script: "${runDockerBuildCommand} --base-image-names-only",
								returnStdout: true
							).split('\n')

							docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
								for (baseImageName in baseImageNames) {
									docker.image(baseImageName).pull()
								}
							}
						}
					}
				}
				stage('build') {
					steps {
						sh "${runDockerBuildCommand}"
					}
				}
				stage('push built image') {
					when {
						expression { SHOULD_PUBLISH_BUILD_IMAGE.toBoolean() }
					}
					steps {
						script {
							dockerHelper.loginAndRunCommand(dockerCredentialId, dockerUrl) {
								final String hostName = helper.resolveUrlHostName(dockerUrl)

								dockerHelper.pushImage(buildImageFullName, "${hostName}/${buildImageFullName}")
								String archImageName = "${hostName}/${dockerRepoName}:${resolveShortArchitectureImageLabel(compilerConfigurationFilepath)}"
								dockerHelper.pushImage(buildImageFullName, archImageName)
								String multiArchImageName = "${hostName}/${dockerRepoName}:${resolveShortImageLabel()}"
								dockerHelper.updateDockerImage(multiArchImageName, archImageName, "${ARCHITECTURE}")
							}
						}
					}
				}
			}
		}
	}
}

Boolean isPublicBuild() {
	return 'release-public' == BUILD_CONFIGURATION
}

Boolean isManualBuild() {
	return null != MANUAL_GIT_BRANCH && '' != MANUAL_GIT_BRANCH && 'null' != MANUAL_GIT_BRANCH
}

String resolveBranchName() {
	return isManualBuild() ? MANUAL_GIT_BRANCH : env.GIT_BRANCH
}

String publicVersion() {
	versionPath = './symbol-mono/jenkins/catapult/server.version.yaml'
	data = readYaml(file: versionPath)
	return data.version
}

String resolveImageRepo() {
	return isPublicBuild() ? 'symbol-server' : 'symbol-server-private'
}

String resolveArchitectureLabel(String compilerConfigurationFilepath) {
	data = readYaml(file: "${compilerConfigurationFilepath}")
	return "-${data.architecture}"
}

String resolveImageLabel(String compilerConfigurationFilepath) {
	String friendlyBranchName = resolveBranchName()
	String compilerVersionName = resolveCompilerVersionName()
	if (0 == friendlyBranchName.indexOf('origin/')) {
		friendlyBranchName = friendlyBranchName.substring(7)
	}

	friendlyBranchName = friendlyBranchName.replaceAll('/', '-')
	architecture = resolveArchitectureLabel(compilerConfigurationFilepath)
	gitHash = "${env.GIT_COMMIT}".substring(0, 8)
	return "${compilerVersionName}-${friendlyBranchName}${architecture}-${gitHash}"
}

String resolveShortImageLabel() {
	String compilerName = resolveCompilerName()
	if (!isPublicBuild()) {
		return compilerName
	}

	versionString = publicVersion()
	return "${compilerName}-${versionString}"
}

String resolveShortArchitectureImageLabel(String compilerConfigurationFilepath) {
	return "${resolveShortImageLabel()}${resolveArchitectureLabel(compilerConfigurationFilepath)}"
}

String resolveCompilerName() {
	return COMPILER_CONFIGURATION.split('-')[0]
}

String resolveCompilerVersionName() {
	String compilerName = resolveCompilerName()
	dir('symbol-mono/jenkins/catapult/compilers')
	{
		compilerVersion = readYaml(file: "${COMPILER_CONFIGURATION}.yaml").version
	}

	return "${compilerName}-${compilerVersion}"
}
