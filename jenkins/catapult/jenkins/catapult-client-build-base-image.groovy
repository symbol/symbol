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

		booleanParam name: 'SHOULD_BUILD_CONAN_LAYER', description: 'true to build conan layer', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName("${OPERATING_SYSTEM}")}"
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
							destImageName = "symbolplatform/symbol-server-build-base:${OPERATING_SYSTEM}-${COMPILER_CONFIGURATION}"

							baseImageDockerfileGeneratorCommand = """
								python3 ./jenkins/catapult/baseImageDockerfileGenerator.py \
									--compiler-configuration jenkins/catapult/configurations/${COMPILER_CONFIGURATION}.yaml \
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
							dockerBuildAndPushLayer('test', "${baseImageDockerfileGeneratorCommand}")
						}
					}
				}
				stage('build conan') {
					when {
						expression { SHOULD_BUILD_CONAN_LAYER.toBoolean() }
					}
					steps {
						script {
							dockerBuildAndPushLayer('conan', "${baseImageDockerfileGeneratorCommand}")
						}
					}
				}
			}
		}
	}
}

void dockerBuildAndPushLayer(String layer, String baseImageDockerfileGeneratorCommand) {
	docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
		destImageName = sh(
			script: "${baseImageDockerfileGeneratorCommand} --layer ${layer} --name-only",
			returnStdout: true
		).trim()

		sh """
			${baseImageDockerfileGeneratorCommand} --layer ${layer} > Dockerfile

			echo "*** LAYER ${layer} => ${destImageName} ***"
			cat Dockerfile
		"""

		docker.build("${destImageName}").push()
	}
}
