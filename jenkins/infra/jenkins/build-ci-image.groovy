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
			choices: ['arm64', 'amd64'],
			description: 'Computer architecture'
		choice name: 'BASE_IMAGE',
			choices: ['lts', 'base', 'latest'],
			description: 'Base image'
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
	}

	agent {
		label """${
			env.ARCHITECTURE = env.ARCHITECTURE ?: 'arm64'
			helper.resolveAgentName(params.OPERATING_SYSTEM, env.ARCHITECTURE, 'medium')
		}"""
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
								Object buildEnvironments = jobHelper.loadBuildBaseImages()
								baseImageName = "${params.OPERATING_SYSTEM}-${params.BASE_IMAGE}"
								buildEnvironment = buildEnvironments[baseImageName]
								dockerFromImage = buildEnvironment.image

								multiArchImageName = "symbolplatform/build-ci:${CI_IMAGE}-${baseImageName}"
								archImageName = "${multiArchImageName}-${env.ARCHITECTURE}"
							}
						}
					}
				}
				stage('print env') {
					steps {
						echo """
									env.GIT_BRANCH: ${env.GIT_BRANCH}
								 MANUAL_GIT_BRANCH: ${env.MANUAL_GIT_BRANCH}
								  OPERATING_SYSTEM: ${env.OPERATING_SYSTEM}
									  ARCHITECTURE: ${env.ARCHITECTURE}

								     destImageName: ${multiArchImageName}
						"""
					}
				}
			}
		}
		stage('checkout') {
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
							String versionArg = getVersionArg(params.CI_IMAGE, buildEnvironment)
							String buildArg = "-f ${CI_IMAGE}.Dockerfile ${versionArg} --build-arg FROM_IMAGE=${dockerFromImage} ."
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

String getVersionArg(String toolName, Object buildEnvironment) {
	String name = 'javascript' == toolName ? 'nodejs' : toolName
	String version = buildEnvironment[name]

	return version ? "--build-arg ${name.toUpperCase()}_VERSION=${version}" : ''
}
