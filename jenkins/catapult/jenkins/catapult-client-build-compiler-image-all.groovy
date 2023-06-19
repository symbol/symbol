pipeline {
	agent {
		label 'ubuntu-small-agent'
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
				stage('gcc prior - amd64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-prior', 'ubuntu', 'amd64')
						}
					}
				}
				stage('gcc latest - amd64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-latest', 'ubuntu', 'amd64')
						}
					}
				}
				stage('gcc [debian] - amd64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-debian', 'debian', 'amd64')
						}
					}
				}
				stage('gcc [fedora] - amd64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-latest', 'fedora', 'amd64')
						}
					}
				}

				stage('clang prior - amd64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('clang-prior', 'ubuntu', 'amd64')
						}
					}
				}
				stage('clang latest - amd64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('clang-latest', 'ubuntu', 'amd64')
						}
					}
				}

				stage('msvc latest - amd64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('msvc-latest', 'windows', 'amd64')
						}
					}
				}
				stage('msvc prior - amd64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('msvc-prior', 'windows', 'amd64')
						}
					}
				}

				stage('gcc prior - arm64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-prior', 'ubuntu', 'arm64')
						}
					}
				}
				stage('gcc latest - arm64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-latest', 'ubuntu', 'arm64')
						}
					}
				}
				stage('gcc [debian] - arm64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-debian', 'debian', 'arm64')
						}
					}
				}
				stage('gcc [fedora] - arm64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('gcc-latest', 'fedora', 'arm64')
						}
					}
				}

				stage('clang prior - arm64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('clang-prior', 'ubuntu', 'arm64')
						}
					}
				}
				stage('clang latest - arm64') {
					steps {
						script {
							dispatchBuildCompilerImageJob('clang-latest', 'ubuntu', 'arm64')
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
