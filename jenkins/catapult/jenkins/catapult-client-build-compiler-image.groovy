pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'COMPILER_CONFIGURATION',
			choices: ['gcc-latest', 'gcc-prior', 'gcc-debian', 'clang-latest', 'clang-prior', 'msvc-latest', 'msvc-prior'],
			description: 'compiler version'
		choice name: 'OPERATING_SYSTEM',
			choices: ['ubuntu', 'fedora', 'debian', 'windows'],
			description: 'operating system'
		booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: false
	}

	agent {
		label "${helper.resolveAgentName("${OPERATING_SYSTEM}")}"
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
								destImageName = sh(
									script: """
										python3 ./jenkins/catapult/baseImageDockerfileGenerator.py \
											--compiler-configuration jenkins/catapult/configurations/${COMPILER_CONFIGURATION}.yaml \
											--operating-system ${OPERATING_SYSTEM} \
											--versions ./jenkins/catapult/versions.properties \
											--layer os \
											--base-name-only
									""",
									returnStdout: true
								).trim()
							}
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
			steps {
				script {
					helper.runStepAndRecordFailure {
						String[] compilerParts = COMPILER_CONFIGURATION.split('-')
						dir("jenkins/catapult/compilers/${OPERATING_SYSTEM}-${compilerParts[0]}")
						{
							String compilerVersion = readYaml(file: "../${COMPILER_CONFIGURATION}.yaml").version
							String buildArg = "--build-arg COMPILER_VERSION=${compilerVersion} ."
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
						"Compiler Image Job Failed for ${currentBuild.fullDisplayName}",
						"Job with ${COMPILER_CONFIGURATION} on ${OPERATING_SYSTEM} has result of ${currentBuild.currentResult} in"
						+ " stage **${env.FAILED_STAGE_NAME}** with message: **${env.FAILURE_MESSAGE}**.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}
