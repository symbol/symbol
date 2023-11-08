pipeline {
	parameters {
		gitParameter branchFilter: 'origin/(.*)', defaultValue: 'dev', name: 'MANUAL_GIT_BRANCH', type: 'PT_BRANCH'
		choice name: 'ARCHITECTURE', choices: ['amd64', 'arm64'], description: 'Computer architecture'
		booleanParam name: 'SHOULD_PUBLISH_JOB_STATUS', description: 'true to publish job status', defaultValue: true
	}

	agent {
		label """${
			env.ARCHITECTURE = env.ARCHITECTURE ?: 'amd64'
			helper.resolveAgentName('ubuntu', env.ARCHITECTURE, 'small')
		}"""
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
						 MANUAL_GIT_BRANCH: ${env.MANUAL_GIT_BRANCH}
							  ARCHITECTURE: ${env.ARCHITECTURE}
				"""
			}
		}

		stage('build servers') {
			parallel {
				stage('gcc (metal) [debian]') {
					steps {
						script {
							dispatchBuildJob('gcc-debian', 'tests-metal', 'debian', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('gcc (westmere)') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildJob('gcc-westmere', 'tests-metal', 'ubuntu', 'amd64')
						}
					}
				}

				stage('gcc (metal) [fedora]') {
					steps {
						script {
							dispatchBuildJob('gcc-latest', 'tests-metal', 'fedora', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('clang prior (metal)') {
					steps {
						script {
							dispatchBuildJob('clang-prior', 'tests-metal', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('clang prior (conan)') {
					steps {
						script {
							dispatchBuildJob('clang-prior', 'tests-conan', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('gcc prior (metal)') {
					steps {
						script {
							dispatchBuildJob('gcc-prior', 'tests-metal', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('gcc prior (conan)') {
					steps {
						script {
							dispatchBuildJob('gcc-prior', 'tests-conan', 'ubuntu', "${env.ARCHITECTURE}")
						}
					}
				}

				stage('msvc prior (metal)') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildJob('msvc-prior', 'tests-metal', 'windows', 'amd64')
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

void dispatchBuildJob(String compilerConfiguration, String buildConfiguration, String operatingSystem, String architecture) {
	build job: 'catapult-client-build-catapult-project', parameters: [
		string(name: 'COMPILER_CONFIGURATION', value: "${compilerConfiguration}"),
		string(name: 'BUILD_CONFIGURATION', value: "${buildConfiguration}"),
		string(name: 'OPERATING_SYSTEM', value: "${operatingSystem}"),
		string(name: 'MANUAL_GIT_BRANCH', value: "${params.MANUAL_GIT_BRANCH}"),
		string(name: 'ARCHITECTURE', value: "${architecture}"),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}
