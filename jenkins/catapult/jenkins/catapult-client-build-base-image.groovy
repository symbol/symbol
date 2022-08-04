pipeline {
	agent {
		label 'ubuntu-xlarge-agent'
	}

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'COMPILER_CONFIGURATION',
			choices: ['gcc-latest', 'gcc-prior', 'gcc-10', 'gcc-westmere', 'clang-latest', 'clang-prior', 'clang-ausan', 'clang-tsan'],
			description: 'compiler configuration'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'fedora', 'debian'],
			description: 'operating system'

		booleanParam name: 'SHOULD_BUILD_CONAN_LAYER', description: 'true to build conan layer', defaultValue: false
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
							dest_image_name = "symbolplatform/symbol-server-build-base:${OPERATING_SYSTEM}-${COMPILER_CONFIGURATION}"

							base_image_dockerfile_generator_command = """
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

								   dest_image_name: ${dest_image_name}
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
							build_and_push_layer('os', "${base_image_dockerfile_generator_command}")
						}
					}
				}
				stage('build boost') {
					steps {
						script {
							build_and_push_layer('boost', "${base_image_dockerfile_generator_command}")
						}
					}
				}
				stage('build deps') {
					steps {
						script {
							build_and_push_layer('deps', "${base_image_dockerfile_generator_command}")
						}
					}
				}
				stage('build test') {
					steps {
						script {
							build_and_push_layer('test', "${base_image_dockerfile_generator_command}")
						}
					}
				}
				stage('build conan') {
					when {
						expression { SHOULD_BUILD_CONAN_LAYER.toBoolean() }
					}
					steps {
						script {
							build_and_push_layer('conan', "${base_image_dockerfile_generator_command}")
						}
					}
				}
			}
		}
	}
}

def build_and_push_layer(layer, base_image_dockerfile_generator_command) {
	docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
		dest_image_name = sh(
			script: "${base_image_dockerfile_generator_command} --layer ${layer} --name-only",
			returnStdout: true
		).trim()

		sh """
			${base_image_dockerfile_generator_command} --layer ${layer} > Dockerfile

			echo "*** LAYER ${layer} => ${dest_image_name} ***"
			cat Dockerfile
		"""

		docker.build("${dest_image_name}").push()
	}
}
