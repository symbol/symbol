pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'COMPILER_CONFIGURATION',
			choices: [
				'gcc-latest',
				'gcc-prior',
				'gcc-debian',
				'gcc-westmere',
				'clang-latest',
				'clang-prior',
				'clang-ausan',
				'clang-tsan',
				'msvc-latest',
				'msvc-prior'
			],
			description: 'compiler configuration'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'fedora', 'debian', 'windows'],
			description: 'operating system'
		choice name: 'ARCHITECTURE',
			choices: ['amd64', 'arm64'],
			description: 'Computer architecture'

		booleanParam name: 'SHOULD_BUILD_CONAN_LAYER', description: 'true to build conan layer', defaultValue: false
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
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
		timeout(time: 3, unit: 'HOURS')
	}

	stages {
		stage('prepare') {
			stages {
				stage('git checkout') {
					steps {
						sh "git checkout ${params.MANUAL_GIT_BRANCH}"
						sh "git reset --hard origin/${params.MANUAL_GIT_BRANCH}"
					}
				}
				stage('prepare variables') {
					steps {
						script {
							destImageName = "symbolplatform/symbol-server-build-base:${OPERATING_SYSTEM}-${COMPILER_CONFIGURATION}"

							baseImageDockerfileGeneratorCommand = """
								python3 ./jenkins/catapult/baseImageDockerfileGenerator.py \
									--compiler-configuration jenkins/catapult/configurations/${ARCHITECTURE}/${COMPILER_CONFIGURATION}.yaml \
									--operating-system ${OPERATING_SYSTEM} \
									--versions ./jenkins/catapult/versions.properties \
							"""
						}
					}
				}
				stage('print env') {
					steps {
						echo """
									env.GIT_BRANCH: ${env.GIT_BRANCH}
								 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}

							COMPILER_CONFIGURATION: ${COMPILER_CONFIGURATION}
								  OPERATING_SYSTEM: ${OPERATING_SYSTEM}
						  SHOULD_BUILD_CONAN_LAYER: ${SHOULD_BUILD_CONAN_LAYER}
									  ARCHITECTURE: ${ARCHITECTURE}

									 destImageName: ${destImageName}
						"""
					}
				}
			}
		}
		stage('build image') {
			stages {
				stage('build os') {
					steps {
						script {
							dockerBuildAndPushLayer('os', "${baseImageDockerfileGeneratorCommand}")
						}
					}
				}
				stage('build boost') {
					steps {
						script {
							dockerBuildAndPushLayer('boost', "${baseImageDockerfileGeneratorCommand}")
						}
					}
				}
				stage('build deps') {
					steps {
						script {
							dockerBuildAndPushLayer('deps', "${baseImageDockerfileGeneratorCommand}")
						}
					}
				}
				stage('build test') {
					steps {
						script {
							final String stageName = 'test'
							dockerBuildAndPushLayer(stageName, "${baseImageDockerfileGeneratorCommand}")
							tagDockerImageForStage(stageName)
						}
					}
				}
				stage('build conan') {
					when {
						expression { SHOULD_BUILD_CONAN_LAYER.toBoolean() }
					}
					steps {
						script {
							final String stageName = 'conan'
							dockerBuildAndPushLayer(stageName, "${baseImageDockerfileGeneratorCommand}")
							tagDockerImageForStage(stageName)
						}
					}
				}
			}
		}
	}
	post {
		unsuccessful {
			script {
				if (env.SHOULD_PUBLISH_FAIL_JOB_STATUS?.toBoolean()) {
					helper.sendDiscordNotification(
						"Catapult Client Base Image Job Failed for ${currentBuild.fullDisplayName}",
						"Job with ${COMPILER_CONFIGURATION} on ${OPERATING_SYSTEM} has result of ${currentBuild.currentResult} in"
						+ " stage **${env.FAILED_STAGE_NAME}** with message: **${env.FAILURE_MESSAGE}**.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}

String resolveImageName(String layer, boolean ignoreArchitecture) {
	command = "${baseImageDockerfileGeneratorCommand} --layer ${layer} --name-only"
	if (ignoreArchitecture) {
		command += ' --ignore-architecture'
	}

	return sh(
			script: command,
			returnStdout: true
	).trim()
}

void dockerBuildAndPushLayer(String layer, String baseImageDockerfileGeneratorCommand) {
	helper.runStepAndRecordFailure {
		destImageName = resolveImageName(layer, false)
		sh """
				${baseImageDockerfileGeneratorCommand} --layer ${layer} > Dockerfile

				echo "*** LAYER ${layer} => ${destImageName} ***"
				cat Dockerfile
			"""

		docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
			docker.build(destImageName).push()
		}
	}
}

void tagDockerImageForStage(String stageName) {
	String destImageName = resolveImageName(stageName, true)
	String archImageName = resolveImageName(stageName, false)
	dockerHelper.tagDockerImage("${OPERATING_SYSTEM}", "${DOCKER_URL}", "${DOCKER_CREDENTIALS_ID}", archImageName, destImageName)
}
