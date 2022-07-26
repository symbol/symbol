pipeline {
	agent {
		label 'windows-xlarge-agent'
	}

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'main', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'COMPILER_CONFIGURATION',
			choices: ['msvc-2022', 'msvc-2019'],
			description: 'compiler configuration'
		choice name: 'OPERATING_SYSTEM',
			choices: ['windows'],
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
							Map compilerNameVersionMap = [ 'msvc-2022': 17, 'msvc-2019': 16]
							compilerVersion = compilerNameVersionMap["${COMPILER_CONFIGURATION}"]
							destImageName = "symbolplatform/symbol-server-build-base:windows-msvc-${compilerVersion}-conan"
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

									 destImageName: ${destImageName}
						"""
					}
				}
			}
		}
		stage('build image') {
			stages {
				stage('compiler image') {
					steps {
						script {
							dir('jenkins/catapult/compilers/windows-msvc') {
								String buildArg = "--build-arg CHANNEL_URL=https://aka.ms/vs/${compilerVersion}/release/channel \
										        --build-arg BUILDTOOLS_URL=https://aka.ms/vs/${compilerVersion}/release/vs_buildtools.exe ."

								docker.build(destImageName, buildArg)
								docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
									docker.image(destImageName).push()
								}
							}
						}
					}
				}
			}
		}
	}
}
