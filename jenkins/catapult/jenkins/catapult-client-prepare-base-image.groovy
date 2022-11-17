pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'fedora', 'debian', 'windows'],
			description: 'operating system'
		choice name: 'IMAGE_TYPE',
			choices: ['release', 'test'],
			description: 'image type'

		booleanParam name: 'SANITIZER_BUILD', description: 'true to build sanitizer', defaultValue: false
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName("${OPERATING_SYSTEM}")}"
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
						   SANITIZER_BUILD: ${SANITIZER_BUILD}
				"""
			}
		}
		stage('prepare Dockerfile') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						properties = readProperties(file: './jenkins/catapult/versions.properties')
						version = properties[params.OPERATING_SYSTEM]

						sanitizer = SANITIZER_BUILD.toBoolean() ? 'Sanitizer' : ''
						filename = "${params.OPERATING_SYSTEM.capitalize()}${params.IMAGE_TYPE.capitalize()}${sanitizer}"
						dockerfileTemplate = "./jenkins/catapult/templates/${filename}BaseImage.Dockerfile"
						dockerfileContents = readFile(file: dockerfileTemplate)
						baseImage = 'windows' == "${OPERATING_SYSTEM}"
								? "mcr.microsoft.com/windows/servercore:ltsc${version}"
								: "${params.OPERATING_SYSTEM}:${version}"
						dockerfileContents = dockerfileContents.replaceAll('\\{\\{BASE_IMAGE\\}\\}', "${baseImage}")

						writeFile(file: 'Dockerfile', text: dockerfileContents)
					}
				}
			}
		}
		stage('print Dockerfile') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						sh '''
							echo '*** Dockerfile ***'
							cat Dockerfile
						'''
					}
				}
			}
		}

		stage('build image') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						dockerImageName = "symbolplatform/symbol-server-${params.IMAGE_TYPE}-base:${params.OPERATING_SYSTEM}"
						if (SANITIZER_BUILD.toBoolean()) {
							dockerImageName += '-sanitizer'
						}

						echo "Docker image name: ${dockerImageName}"
						dockerImage = docker.build(dockerImageName)
						docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
							dockerImage.push()
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
						"Catapult Client Prepare Base Image Job Failed for ${currentBuild.fullDisplayName}",
						"Job creating **${env.IMAGE_TYPE}** image on ${env.OPERATING_SYSTEM} has result of ${currentBuild.currentResult}"
						+ " in stage **${env.FAILED_STAGE_NAME}** with message: **${env.FAILURE_MESSAGE}**.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}
