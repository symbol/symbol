pipeline {
	agent {
		label 'ubuntu-xlarge-agent'
	}

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'COMPILER_VERSION',
			choices: ['gcc-latest', 'gcc-prior', 'gcc-10', 'clang-latest', 'clang-prior'],
			description: 'compiler version'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'fedora', 'debian'],
			description: 'operating system'
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
							destImageName = "symbolplatform/symbol-server-compiler:${OPERATING_SYSTEM}-${COMPILER_VERSION}"
						}
					}
				}
				stage('print env') {
					steps {
						echo """
									env.GIT_BRANCH: ${env.GIT_BRANCH}
								 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}

								  COMPILER_VERSION: ${COMPILER_VERSION}
								  OPERATING_SYSTEM: ${OPERATING_SYSTEM}

								     destImageName: ${destImageName}
						"""
					}
				}
			}
		}
		stage('build image') {
			steps {
				script {
					String compilerParts = COMPILER_VERSION.split('-')
					dir("jenkins/catapult/compilers/${OPERATING_SYSTEM}-${compilerParts[0]}")
					{
						String buildArg = "--build-arg COMPILER_VERSION=${compilerParts[1]} ."
						docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
							docker.build(destImageName, buildArg).push()
						}
					}
				}
			}
		}
	}
}

