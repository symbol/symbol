pipeline {
	agent any

	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		booleanParam name: 'SHOULD_PUBLISH_JOB_STATUS', description: 'true to publish job status', defaultValue: true
	}

	options {
		ansiColor('css')
		timestamps()
	}

	triggers {
		// second of the month
		cron('H 0 2 * *')
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

		stage('build base images') {
			parallel {
				stage('gcc prior - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-prior', 'ubuntu', true, 'amd64')
						}
					}
				}
				stage('gcc latest - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-latest', 'ubuntu', true, 'amd64')
						}
					}
				}
				stage('gcc 10 [debian] - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-debian', 'debian', false, 'amd64')
						}
					}
				}
				stage('gcc westmere - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-westmere', 'ubuntu', true, 'amd64')
						}
					}
				}
				stage('gcc [fedora] - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-latest', 'fedora', false, 'amd64')
						}
					}
				}

				stage('clang prior - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-prior', 'ubuntu', true, 'amd64')
						}
					}
				}
				stage('clang latest - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-latest', 'ubuntu', true, 'amd64')
						}
					}
				}

				stage('clang ausan - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-ausan', 'ubuntu', false, 'amd64')
						}
					}
				}
				stage('clang tsan - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-tsan', 'ubuntu', false, 'amd64')
						}
					}
				}

				stage('msvc latest - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('msvc-latest', 'windows', true, 'amd64')
						}
					}
				}
				stage('msvc prior - amd64') {
					steps {
						script {
							dispatchBuildBaseImageJob('msvc-prior', 'windows', true, 'amd64')
						}
					}
				}

				stage('release base image - amd64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('release', 'ubuntu', 'amd64')
						}
					}
				}

				stage('test base image - amd64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'ubuntu', 'amd64')
						}
					}
				}
				stage('test base image [debian] - amd64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'debian', 'amd64')
						}
					}
				}
				stage('test base image [fedora] - amd64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'fedora', 'amd64')
						}
					}
				}
				stage('test base image [windows] - amd64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'windows', 'amd64')
						}
					}
				}

				stage('gcc prior - arm64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-prior', 'ubuntu', true, 'arm64')
						}
					}
				}
				stage('gcc latest - arm64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-latest', 'ubuntu', true, 'arm64')
						}
					}
				}
				stage('gcc 10 [debian] - arm64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-debian', 'debian', false, 'arm64')
						}
					}
				}
				stage('gcc [fedora] - arm64') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-latest', 'fedora', false, 'arm64')
						}
					}
				}

				stage('clang prior - arm64') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-prior', 'ubuntu', true, 'arm64')
						}
					}
				}
				stage('clang latest - arm64') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-latest', 'ubuntu', true, 'arm64')
						}
					}
				}

				stage('clang ausan - arm64') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-ausan', 'ubuntu', false, 'arm64')
						}
					}
				}
				stage('clang tsan - arm64') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-tsan', 'ubuntu', false, 'arm64')
						}
					}
				}

				stage('release base image - arm64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('release', 'ubuntu', 'arm64')
						}
					}
				}
				stage('test base image - arm64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'ubuntu', 'arm64')
						}
					}
				}
				stage('test base image [fedora] - arm64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'fedora', 'arm64')
						}
					}
				}
				stage('test base image [debian] - arm64') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'debian', 'arm64')
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
						':confetti_ball: Catapult Client All Image Job Successfully completed',
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
						":confused: Catapult Client All Image Job Failed for ${currentBuild.fullDisplayName}",
						"At least an image job failed for Build#${env.BUILD_NUMBER} with a result of ${currentBuild.currentResult}.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}

void dispatchBuildBaseImageJob(String compilerConfiguration, String operatingSystem, Boolean shouldBuildConanLayer, String architecture) {
	build job: 'catapult-client-build-base-image', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compilerConfiguration}"),
		string(name: 'OPERATING_SYSTEM', value: "${operatingSystem}"),
		string(name: 'SHOULD_BUILD_CONAN_LAYER', value: "${shouldBuildConanLayer}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		string(name: 'ARCHITECTURE', value: "${architecture}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}

void dispatchPrepareBaseImageJob(String imageType, String operatingSystem, String architecture) {
	build job: 'catapult-client-prepare-base-image', parameters: [
		string(name: 'IMAGE_TYPE', value: "${imageType}"),
		string(name: 'OPERATING_SYSTEM', value: "${operatingSystem}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		string(name: 'ARCHITECTURE', value: "${architecture}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}
