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

	stages {
		stage('print env') {
			steps {
				echo """
							env.GIT_BRANCH: ${env.GIT_BRANCH}
						 MANUAL_GIT_BRANCH: ${MANUAL_GIT_BRANCH}
				"""
			}
		}

		stage('build servers') {
			parallel {
				stage('gcc (metal) [debian]') {
					steps {
						script {
							dispatchBuildJob('gcc-debian', 'tests-metal', 'debian')
						}
					}
				}

				stage('gcc (westmere)') {
					steps {
						script {
							dispatchBuildJob('gcc-westmere', 'tests-metal', 'ubuntu')
						}
					}
				}

				stage('gcc (metal) [fedora]') {
					steps {
						script {
							dispatchBuildJob('gcc-latest', 'tests-metal', 'fedora')
						}
					}
				}

				stage('clang prior (metal)') {
					steps {
						script {
							dispatchBuildJob('clang-prior', 'tests-metal', 'ubuntu')
						}
					}
				}

				stage('clang prior (conan)') {
					steps {
						script {
							dispatchBuildJob('clang-prior', 'tests-conan', 'ubuntu')
						}
					}
				}

				stage('gcc prior (metal)') {
					steps {
						script {
							dispatchBuildJob('gcc-prior', 'tests-metal', 'ubuntu')
						}
					}
				}

				stage('gcc prior (conan)') {
					steps {
						script {
							dispatchBuildJob('gcc-prior', 'tests-conan', 'ubuntu')
						}
					}
				}

				stage('msvc prior (metal)') {
					steps {
						script {
							dispatchBuildJob('msvc-prior', 'tests-metal', 'windows')
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
						':partying_face: Catapult Client Weekly Job Successfully completed',
						'All is good with the client',
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
						":face_with_monocle: Catapult Client Weekly Job Failed for ${currentBuild.fullDisplayName}",
						"At least one job failed for Build#${env.BUILD_NUMBER} with a result of ${currentBuild.currentResult}.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}

void dispatchBuildJob(String compilerConfiguration, String buildConfiguration, String operatingSystem) {
	build job: 'catapult-client-build-catapult-project', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compilerConfiguration}"),
		string(name: 'BUILD_CONFIGURATION', value: "${buildConfiguration}"),
		string(name: 'OPERATING_SYSTEM', value: "${operatingSystem}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}
