pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'ARCHITECTURE', choices: ['arm64', 'amd64'], description: 'Computer architecture'
		booleanParam name: 'SHOULD_PUBLISH_JOB_STATUS', description: 'true to publish job status', defaultValue: true
	}

	agent {
		label """${
			env.ARCHITECTURE = env.ARCHITECTURE ?: 'arm64'
			helper.resolveAgentName('ubuntu', "${env.ARCHITECTURE}", 'small')
		}"""
	}

	options {
		ansiColor('css')
		timestamps()
	}

	triggers {
		// first and second of the month
		cron('H 0 1,2 * *')
	}

	stages {
		stage('override architecture') {
			when {
				triggeredBy 'TimerTrigger'
			}
			steps {
				script {
					// even days are amd64, odd days are arm64
					env.ARCHITECTURE = helper.determineArchitecture()
				}
			}
		}
		stage('print env') {
			steps {
				echo """
							env.GIT_BRANCH: ${env.GIT_BRANCH}
						 MANUAL_GIT_BRANCH: ${env.MANUAL_GIT_BRANCH}
							  ARCHITECTURE: ${env.ARCHITECTURE}
				"""
			}
		}

		stage('build compiler images') {
			parallel {
				stage('gcc prior') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-prior', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}
				stage('gcc latest') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-latest', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}
				stage('gcc [debian]') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-debian', 'debian', "${env.ARCHITECTURE}")
						}
					}
				}
				stage('gcc [fedora]') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-latest', 'fedora', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('clang prior') {
					steps {
						script {
							dispatchBuildCompilerImageJob('clang-prior', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}
				stage('clang latest') {
					steps {
						script {
							dispatchBuildCompilerImageJob('clang-latest', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('msvc latest') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildCompilerImageJob('msvc-latest', 'windows', 'amd64')
						}
					}
				}
				stage('msvc prior') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildCompilerImageJob('msvc-prior', 'windows', 'amd64')
						}
					}
				}
			}
		}
	}
	post {
		success {
			script {
				if (env.SHOULD_PUBLISH_JOB_STATUS?.toBoolean()) {
					helper.sendDiscordNotification(
						':confetti_ball: Compiler Image All Job Successfully completed',
						'Not much to see here, all is good',
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
		unsuccessful {
			script {
				if (env.SHOULD_PUBLISH_JOB_STATUS?.toBoolean()) {
					helper.sendDiscordNotification(
						":worried: Compiler Image All Job Failed for ${currentBuild.fullDisplayName}",
						"At least one job failed for Build#${env.BUILD_NUMBER} which has a result of ${currentBuild.currentResult}.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}

void dispatchBuildCompilerImageJob(String compilerConfiguration, String operatingSystem, String architecture) {
	build job: 'catapult-client-build-compiler-image', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compilerConfiguration}"),
		string(name: 'OPERATING_SYSTEM', value: "${operatingSystem}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		string(name: 'ARCHITECTURE', value: "${architecture}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}
