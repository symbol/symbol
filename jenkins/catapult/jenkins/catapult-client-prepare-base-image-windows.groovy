pipeline {
	agent {
		label 'windows-agent'
	}

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'OPERATING_SYSTEM',
			choices: ['windows'],
			description: 'operating system'
		choice name: 'IMAGE_TYPE',
			choices: ['test'],
			description: 'image type'
	}

	options {
		ansiColor('css')
		timestamps()
	}

	environment {
		DOCKER_URL = 'https://registry.hub.docker.com'
		DOCKER_CREDENTIALS_ID = 'docker-hub-token-symbolserverbot'
	}

	stages {
		stage('print env') {
			steps {
				echo """
							env.GIT_BRANCH: ${env.GIT_BRANCH}
						 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}

						  OPERATING_SYSTEM: ${OPERATING_SYSTEM}
								IMAGE_TYPE: ${IMAGE_TYPE}
				"""
			}
		}
		stage('prepare Dockerfile') {
			steps {
				script {
					properties = readProperties(file: './jenkins/catapult/versions.properties')
					version = properties[params.OPERATING_SYSTEM]

					dockerfileTemplate = "./jenkins/catapult/templates/${params.OPERATING_SYSTEM.capitalize()}${params.IMAGE_TYPE.capitalize()}BaseImage.Dockerfile"
					dockerfileContents = readFile(file: dockerfileTemplate)
					dockerfileContents = dockerfileContents.replaceAll('\\{\\{BASE_IMAGE\\}\\}', "mcr.microsoft.com/windows/servercore:ltsc${version}")

					writeFile(file: 'Dockerfile', text: dockerfileContents)
				}
			}
		}
		stage('print Dockerfile') {
			steps {
				sh '''
					echo '*** Dockerfile ***'
					cat Dockerfile
				'''
			}
		}
		stage('build image') {
			steps {
				script {
					docker_image = docker.build "symbolplatform/symbol-server-${params.IMAGE_TYPE}-base:${params.OPERATING_SYSTEM}"
					docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
						docker_image.push()
					}
				}
			}
		}
	}
}
