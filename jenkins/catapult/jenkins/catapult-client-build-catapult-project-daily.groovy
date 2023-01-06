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
				stage('gcc latest (conan)') {
					steps {
						script {
							dispatchUbuntuBuildJob('gcc-latest', 'tests-conan')
						}
					}
				}
				stage('gcc latest (metal)') {
					steps {
						script {
							dispatchUbuntuBuildJob('gcc-latest', 'tests-metal')
						}
					}
				}

				stage('clang latest (conan)') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-latest', 'tests-conan')
						}
					}
				}
				stage('clang latest (metal)') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-latest', 'tests-metal')
						}
					}
				}

				stage('clang ausan') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-ausan', 'tests-metal')
						}
					}
				}
				stage('clang tsan') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-tsan', 'tests-metal')
						}
					}
				}
				stage('clang diagnostics') {
					steps {
						script {
							dispatchUbuntuBuildJob('clang-latest', 'tests-diagnostics')
						}
					}
				}
				stage('code coverage (gcc latest)') {
					steps {
						script {
							dispatchUbuntuBuildJob('gcc-code-coverage', 'tests-metal')
						}
					}
				}
				stage('msvc latest (conan)') {
					steps {
						script {
							dispatchBuildJob('msvc-latest', 'tests-conan', 'windows')
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
						':tada: Catapult Client Daily Job Successfully completed',
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
						":scream_cat: Catapult Client Daily Job Failed for ${currentBuild.fullDisplayName}",
						"At least one daily job failed for Build#${env.BUILD_NUMBER} with a result of ${currentBuild.currentResult}.",
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

void dispatchUbuntuBuildJob(String compilerConfiguration, String buildConfiguration) {
	dispatchBuildJob(compilerConfiguration, buildConfiguration, 'ubuntu')
}
