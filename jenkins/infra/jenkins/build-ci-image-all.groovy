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
		// saturday and sunday of the week
		cron('H H * * 6,7')
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

		stage('build ci images') {
			parallel {
				stage('cpp') {
					steps {
						script {
							dispatchBuildCiImageJob('cpp')
						}
					}
				}
				stage('java') {
					steps {
						script {
							dispatchBuildCiImageJob('java')
						}
					}
				}

				stage('javascript') {
					steps {
						script {
							dispatchBuildCiImageJob('javascript')
						}
					}
				}
				stage('javascript - windows') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildCiImageJob('javascript', 'lts', 'windows')
						}
					}
				}
				stage('javascript - fedora') {
					steps {
						script {
							dispatchBuildCiImageJob('javascript', 'lts', 'fedora')
						}
					}
				}

				stage('linter') {
					steps {
						script {
							dispatchBuildCiImageJob('linter')
						}
					}
				}
				stage('postgres') {
					steps {
						script {
							dispatchBuildCiImageJob('postgres')
						}
					}
				}

				stage('python') {
					steps {
						script {
							dispatchBuildCiImageJob('python')
						}
					}
				}
				stage('python - ubuntu base') {
					steps {
						script {
							dispatchBuildCiImageJob('python', 'base')
						}
					}
				}
				stage('python - ubuntu latest') {
					steps {
						script {
							dispatchBuildCiImageJob('python', 'latest')
						}
					}
				}
				stage('python - windows') {
					when {
						expression {
							helper.isAmd64Architecture(env.ARCHITECTURE)
						}
					}
					steps {
						script {
							dispatchBuildCiImageJob('python', 'lts', 'windows')
						}
					}
				}
				stage('python - fedora') {
					steps {
						script {
							dispatchBuildCiImageJob('python', 'lts', 'fedora')
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
						':confetti_ball: CI Image All Job Successfully completed',
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
						":worried: CI Image All Job Failed for ${currentBuild.fullDisplayName}",
						"At least one job failed for Build#${env.BUILD_NUMBER} which has a result of ${currentBuild.currentResult}.",
						env.BUILD_URL,
						currentBuild.currentResult
					)
				}
			}
		}
	}
}

void dispatchBuildCiImageJob(String ciImage, String baseImage = 'lts', String operatingSystem = 'ubuntu') {
	build job: 'build-ci-image', parameters: [
		string(name: 'OPERATING_SYSTEM', value: operatingSystem),
		string(name: 'CI_IMAGE', value: ciImage),
		string(name: 'MANUAL_GIT_BRANCH', value: params.MANUAL_GIT_BRANCH),
		string(name: 'ARCHITECTURE', value: env.ARCHITECTURE),
		string(name: 'BASE_IMAGE', value: baseImage),
		booleanParam(
			name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS',
			value: "${!env.SHOULD_PUBLISH_JOB_STATUS || env.SHOULD_PUBLISH_JOB_STATUS.toBoolean()}"
		)
	]
}
