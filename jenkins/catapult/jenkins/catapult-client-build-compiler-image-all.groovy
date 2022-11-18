pipeline {
	agent {
		label 'ubuntu-agent'
	}

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		booleanParam name: 'SHOULD_PUBLISH_JOB_STATUS', description: 'true to publish job status', defaultValue: true
	}

	options {
		ansiColor('css')
		timestamps()
	}

	triggers {
		// first of the month
		cron('H 0 1 * *')
	}

	stages {
		stage('print env') {
			steps {
				echo """
							env.GIT_BRANCH: ${env.GIT_BRANCH}
						 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}
				"""
			}
		}

		stage('build compiler images') {
			parallel {
				stage('gcc prior') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-prior', 'ubuntu')
						}
					}
				}
				stage('gcc latest') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-latest', 'ubuntu')
						}
					}
				}
				stage('gcc [debian]') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-debian', 'debian')
						}
					}
				}
				stage('gcc [fedora]') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-latest', 'fedora')
						}
					}
				}

				stage('clang prior') {
					steps {
						script {
							dispatchBuildCompilerImageJob('clang-prior', 'ubuntu')
						}
					}
				}
				stage('clang latest') {
					steps {
						script {
							dispatchBuildCompilerImageJob('clang-latest', 'ubuntu')
						}
					}
				}

				stage('msvc latest') {
					steps {
						script {
							dispatchBuildCompilerImageJob('msvc-latest', 'windows')
						}
					}
				}
				stage('msvc prior') {
					steps {
						script {
							dispatchBuildCompilerImageJob('msvc-prior', 'windows')
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

void dispatchBuildCompilerImageJob(String compilerConfiguration, String operatingSystem) {
	build job: 'Symbol/server-pipelines/catapult-client-build-compiler-image', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compilerConfiguration}"),
		string(name: 'OPERATING_SYSTEM', value: "${operatingSystem}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}
