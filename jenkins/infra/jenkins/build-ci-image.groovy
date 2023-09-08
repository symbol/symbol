pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'windows'],
			description: 'operating system'
		choice name: 'CI_IMAGE',
			choices: ['cpp', 'java', 'javascript', 'linter', 'postgres', 'python'],
			description: 'continuous integration image'
		choice name: 'ARCHITECTURE',
			choices: ['amd64', 'arm64'],
			description: 'Computer architecture'
		choice name: 'BASE_IMAGE',
			choices: ['lts', 'base', 'latest'],
			description: 'Base image'
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName(params.OPERATING_SYSTEM, params.ARCHITECTURE, 'medium')}"
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
								Object buildEnvironment = jobHelper.loadBuildBaseImages()
								baseImageName = "${params.OPERATING_SYSTEM}-${params.BASE_IMAGE}"
								dockerFromImage = buildEnvironment[baseImageName]

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
								  OPERATING_SYSTEM: ${OPERATING_SYSTEM}
									  ARCHITECTURE: ${ARCHITECTURE}

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
					runScript "git reset --hard origin/${env.MANUAL_GIT_BRANCH}"
				}
			}
		}
		stage('build image') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						dir("jenkins/docker/${params.OPERATING_SYSTEM}")
						{
							String buildArg = "-f ${CI_IMAGE}.Dockerfile --build-arg FROM_IMAGE=${dockerFromImage} ."
							docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
								docker.build(archImageName, buildArg).push()
							}
							dockerHelper.tagDockerImage(
								params.OPERATING_SYSTEM,
								"${env.DOCKER_URL}",
								"${env.DOCKER_CREDENTIALS_ID}",
								archImageName,
								multiArchImageName
							)
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
