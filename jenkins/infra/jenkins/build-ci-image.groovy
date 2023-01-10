pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'CI_IMAGE',
			choices: ['cpp', 'java', 'javascript', 'linter', 'postgres', 'python'],
			description: 'continuous integration image'
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
	}

	agent {
		label 'ubuntu-agent'
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
								destImageName = "symbolplatform/build-ci:${CI_IMAGE}"
							}
						}
					}
				}
				stage('print env') {
					steps {
						echo """
									env.GIT_BRANCH: ${env.GIT_BRANCH}
								 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}

								     destImageName: ${destImageName}
						"""
					}
				}
			}
		}
		stage('build image') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						dir('jenkins/docker')
						{
							String buildArg = "-f ${CI_IMAGE}.Dockerfile ."
							docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
								docker.build(destImageName, buildArg).push()
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
