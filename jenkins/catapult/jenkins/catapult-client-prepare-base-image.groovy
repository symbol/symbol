pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'fedora', 'debian', 'windows'],
			description: 'operating system'
		choice name: 'IMAGE_TYPE',
			choices: ['release', 'test'],
			description: 'image type'
		choice name: 'ARCHITECTURE',
			choices: ['amd64', 'arm64'],
			description: 'Computer architecture'

		booleanParam name: 'SANITIZER_BUILD', description: 'true to build sanitizer', defaultValue: false
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName("${OPERATING_SYSTEM}", "${ARCHITECTURE}", 'medium')}"
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
							  ARCHITECTURE: ${ARCHITECTURE}
						   SANITIZER_BUILD: ${SANITIZER_BUILD}
				"""
			}
		}
		stage('git checkout') {
			steps {
				sh "git checkout ${params.MANUAL_GIT_BRANCH}"
				sh "git reset --hard origin/${params.MANUAL_GIT_BRANCH}"
			}
		}
		stage('prepare Dockerfile') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						properties = readProperties(file: './jenkins/catapult/versions.properties')
						version = properties[params.OPERATING_SYSTEM]

						sanitizer = SANITIZER_BUILD.toBoolean() ? 'Sanitizer' : ''
						baseImage = SANITIZER_BUILD.toBoolean()
							? sh(script: """
								python3 ./jenkins/catapult/baseImageDockerfileGenerator.py \
									--compiler-configuration jenkins/catapult/configurations/${ARCHITECTURE}/clang-latest.yaml \
									--operating-system ${OPERATING_SYSTEM} \
									--versions ./jenkins/catapult/versions.properties \
									--layer os \
									--base-name-only
							""",
								returnStdout: true
							).trim()
							: 'windows' == "${OPERATING_SYSTEM}"
								? 'mcr.microsoft.com/powershell:latest'
								: "${params.OPERATING_SYSTEM}:${version}"
						operatingSystem = 'debian' == "${params.OPERATING_SYSTEM}" ? 'ubuntu' : "${params.OPERATING_SYSTEM}"
						filename = "${operatingSystem.capitalize()}${params.IMAGE_TYPE.capitalize()}${sanitizer}"
						dockerfile = "./jenkins/catapult/templates/${filename}BaseImage.Dockerfile"
					}
				}
			}
		}
		stage('print Dockerfile') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						sh """
							echo '*** Dockerfile ***'
							cat ${dockerfile}
						"""
					}
				}
			}
		}

		stage('build image') {
			steps {
				script {
					helper.runStepAndRecordFailure {
						String destImageName = "symbolplatform/symbol-server-${params.IMAGE_TYPE}-base:${params.OPERATING_SYSTEM}"
						if (SANITIZER_BUILD.toBoolean()) {
							destImageName += '-sanitizer'
						}

						String archImageName = destImageName + "-${ARCHITECTURE}"

						echo "Docker image name: ${archImageName}"
						echo "Dockerfile name: ${dockerfile}"
						echo "Base image name: ${baseImage}"

						dockerImage = docker.build(archImageName, "--file ${dockerfile} --build-arg FROM_IMAGE=${baseImage} .")
						docker.withRegistry(DOCKER_URL, DOCKER_CREDENTIALS_ID) {
							dockerImage.push()
						}
						dockerHelper.tagDockerImage("${OPERATING_SYSTEM}", "${DOCKER_URL}", "${DOCKER_CREDENTIALS_ID}", archImageName, destImageName)
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
