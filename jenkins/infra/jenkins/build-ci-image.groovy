pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'CI_IMAGE',
			choices: ['cpp', 'java', 'javascript', 'linter', 'postgres', 'python'],
			description: 'continuous integration image'
		choice name: 'ARCHITECTURE',
			choices: ['amd64', 'arm64'],
			description: 'Computer architecture'
		choice name: 'BASE_IMAGE',
			choices: ['ubuntu:22.04', 'ubuntu:20.04', 'ubuntu:23.04'],
			description: 'Base image'
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName('ubuntu', "${ARCHITECTURE}", 'medium')}"
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
							helper.runStepAndRecordFailure {
								baseImageName = "${BASE_IMAGE}".replace(':', '-')
								multiArchImageName = "symbolplatform/build-ci:${CI_IMAGE}-${baseImageName}"
								archImageName = "${multiArchImageName}-${ARCHITECTURE}"
							}
						}
					}
				}
				stage('print env') {
					steps {
						echo """
									env.GIT_BRANCH: ${env.GIT_BRANCH}
								 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}

								     destImageName: ${multiArchImageName}
						"""
					}
				}
			}
		}
		stage('checkout') {
			when {
				triggeredBy 'UserIdCause'
			}
			steps {
				script {
					sh "git reset --hard origin/${env.MANUAL_GIT_BRANCH}"
				}
			}
		}
		stage('build image') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						dir('jenkins/docker')
						{
							String buildArg = "-f ${CI_IMAGE}.Dockerfile --build-arg FROM_IMAGE=${BASE_IMAGE} ."
							dockerHelper.loginAndRunCommand(DOCKER_CREDENTIALS_ID) {
								dockerHelper.dockerBuildAndPushImage(archImageName, buildArg)
								dockerHelper.updateDockerImage(multiArchImageName, archImageName, "${ARCHITECTURE}")
							}
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
						"CI Image Job Failed for ${currentBuild.fullDisplayName}",
						"Job for ${CI_IMAGE} has result of ${currentBuild.currentResult} in"
						+ " stage **${env.FAILED_STAGE_NAME}** with message: **${env.FAILURE_MESSAGE}**.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}
