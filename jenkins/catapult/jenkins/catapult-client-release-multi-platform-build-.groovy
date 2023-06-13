pipeline {
	agent {
		label 'ubuntu-xlarge-agent'
	}

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

		booleanParam name: 'SHOULD_PUBLISH_BUILD_IMAGE', description: 'true to publish build image', defaultValue: false
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
				stage('prepare variables') {
					steps {
						script {
							fullyQualifiedUser = sh(
								script: 'echo "$(id -u):$(id -g)"',
								returnStdout: true
							).trim()

							imageLabel = resolveImageLabel()
							buildImageFullName = "symbolplatform/${resolveImageRepo()}:${imageLabel}"
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

						SHOULD_PUBLISH_BUILD_IMAGE: ${SHOULD_PUBLISH_BUILD_IMAGE}

								fullyQualifiedUser: ${fullyQualifiedUser}
										imageLabel: ${imageLabel}
								buildImageFullName: ${buildImageFullName}
						"""
					}
				}
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
			}
		}
		stage('build') {
			stages {
				stage('prepare variables') {
					steps {
						script {
							runDockerBuildCommand = """
								python3 symbol-mono/jenkins/catapult/runDockerBuild.py \
									--compiler-configuration symbol-mono/jenkins/catapult/configurations/${COMPILER_CONFIGURATION}.yaml \
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
							shortLabel = resolveShortImageLabel()
							docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
								builtImage = docker.image(buildImageFullName)
								builtImage.push()
								builtImage.push("${shortLabel}")
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

String resolveArchitectureLabel() {
	data = readYaml(file: "./symbol-mono/jenkins/catapult/configurations/${COMPILER_CONFIGURATION}.yaml")
	architecture = data.architecture
	return 'skylake' == architecture ? '' : "-${architecture}"
}

String resolveImageLabel() {
	friendlyBranchName = resolveBranchName()
	if (0 == friendlyBranchName.indexOf('origin/')) {
		friendlyBranchName = friendlyBranchName.substring(7)
	}

	friendlyBranchName = friendlyBranchName.replaceAll('/', '-')
	architecture = resolveArchitectureLabel()
	gitHash = "${env.GIT_COMMIT}".substring(0, 8)
	return "${COMPILER_CONFIGURATION}-${friendlyBranchName}${architecture}-${gitHash}"
}

String resolveShortImageLabel() {
	architecture = resolveArchitectureLabel()
	if (!isPublicBuild()) {
		return "${COMPILER_CONFIGURATION}${architecture}"
	}

	versionString = publicVersion()
	return "${COMPILER_CONFIGURATION}-${versionString}${architecture}"
}
