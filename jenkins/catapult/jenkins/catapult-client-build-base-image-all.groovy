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
		// third and fourth of the month
		cron('H 0 3,4 * *')
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

		stage('build base images') {
			parallel {
				stage('gcc prior') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-prior', 'ubuntu', true, "${env.ARCHITECTURE}")
						}
					}
				}
				stage('gcc latest') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-latest', 'ubuntu', true, "${env.ARCHITECTURE}")
						}
					}
				}
				stage('gcc 10 [debian]') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-debian', 'debian', false, "${env.ARCHITECTURE}")
						}
					}
				}
				stage('gcc westmere') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-westmere', 'ubuntu', true, 'amd64')
						}
					}
				}
				stage('gcc [fedora]') {
					steps {
						script {
							dispatchBuildBaseImageJob('gcc-latest', 'fedora', false, "${env.ARCHITECTURE}")
						}
					}
				}

				stage('clang prior') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-prior', 'ubuntu', true, "${env.ARCHITECTURE}")
						}
					}
				}
				stage('clang latest') {
					steps {
						script {
							dispatchBuildBaseImageJob('clang-latest', 'ubuntu', true, "${env.ARCHITECTURE}")
						}
					}
				}

				stage('clang ausan') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildBaseImageJob('clang-ausan', 'ubuntu', false, 'amd64')
						}
					}
				}
				stage('clang tsan') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildBaseImageJob('clang-tsan', 'ubuntu', false, 'amd64')
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
							dispatchBuildBaseImageJob('msvc-latest', 'windows', true, 'amd64')
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
							dispatchBuildBaseImageJob('msvc-prior', 'windows', true, 'amd64')
						}
					}
				}

				stage('release base image') {
					steps {
						script {
							dispatchPrepareBaseImageJob('release', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('test base image') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}
				stage('san test base image') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'ubuntu', 'amd64', true)
						}
					}
				}
				stage('test base image [debian]') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'debian', "${env.ARCHITECTURE}")
						}
					}
				}
				stage('test base image [fedora]') {
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'fedora', "${env.ARCHITECTURE}")
						}
					}
				}
				stage('test base image [windows]') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchPrepareBaseImageJob('test', 'windows', 'amd64')
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
		booleanParam(name: 'SHOULD_BUILD_CONAN_LAYER', value: shouldBuildConanLayer),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		string(name: 'ARCHITECTURE', value: "${architecture}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}

void dispatchPrepareBaseImageJob(String imageType, String operatingSystem, String architecture, boolean sanitizerBuild = false) {
	build job: 'catapult-client-prepare-base-image', parameters: [
		string(name: 'IMAGE_TYPE', value: "${imageType}"),
		string(name: 'OPERATING_SYSTEM', value: "${operatingSystem}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		string(name: 'ARCHITECTURE', value: "${architecture}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: !env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()
		),
		booleanParam(name: 'SANITIZER_BUILD', value: sanitizerBuild)
	]
}
