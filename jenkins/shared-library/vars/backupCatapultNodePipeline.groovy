// groovylint-disable-next-line MethodSize
void call(Closure body) {
	Map jenkinsfileParams = [:]
	body.resolveStrategy = Closure.DELEGATE_FIRST
	body.delegate = jenkinsfileParams
	body()

	pipeline {
		parameters {
			booleanParam name: 'SHOULD_PUBLISH_FAIL_JOB_STATUS', description: 'true to publish job status if failed', defaultValue: true
		}

		agent {
			label "${jenkinsfileParams.agentName}"
		}

		triggers {
			cron('@weekly')
		}

		options {
			ansiColor('css')
			timestamps()
		}

		stages {
			stage('Shutdown server') {
				steps {
					script {
						helper.runStepRelativeToPackageRoot jenkinsfileParams.catapultPath.toString(), {
							sh 'docker compose down --timeout 300'
						}
					}
				}
			}
			stage('backup data') {
				steps {
					script {
						helper.runStepRelativeToPackageRoot jenkinsfileParams.catapultPath.toString(), {
							sh "tar -cvzf ${jenkinsfileParams.backupFileName} ${jenkinsfileParams.backupPath.join(' ')} > /dev/null"
						}
					}
				}
			}
			stage('start server') {
				steps {
					script {
						helper.runStepRelativeToPackageRoot jenkinsfileParams.catapultPath.toString(), {
							sh 'docker compose up --detach'
						}
					}
				}
			}
			stage('upload weekly data to s3') {
				steps {
					script {
						helper.runStepRelativeToPackageRoot jenkinsfileParams.catapultPath.toString(), {
							sh "aws s3 cp ${jenkinsfileParams.backupFileName} s3://catapultmainnetdata/weekly/ > /dev/null"
							sh "aws s3api put-object-acl --bucket catapultmainnetdata --key weekly/${jenkinsfileParams.backupFileName} --acl public-read"
						}
					}
				}
			}
			stage('upload monthly data to s3') {
				when { expression { return 7 >= helper.dayOfMonth() } }  // first week of the month
				steps {
					script {
						helper.runStepRelativeToPackageRoot jenkinsfileParams.catapultPath.toString(), {
							sh "aws s3 cp ${jenkinsfileParams.backupFileName} s3://catapultmainnetdata/monthly/ > /dev/null"
							sh "aws s3api put-object-acl --bucket catapultmainnetdata --key monthly/${jenkinsfileParams.backupFileName} --acl public-read"
						}
					}
				}
			}
			stage('cleanup backup file') {
				steps {
					script {
						helper.runStepRelativeToPackageRoot jenkinsfileParams.catapultPath.toString(), {
							sh "rm -f ${jenkinsfileParams.backupFileName}"
						}
					}
				}
			}
		}
		post {
			failure {
				dir("${jenkinsfileParams.catapultPath}") {
					sh 'docker compose up --detach'
					sh "rm -f ${jenkinsfileParams.backupFileName}"
				}
			}
			unsuccessful {
				script {
					if (env.SHOULD_PUBLISH_FAIL_JOB_STATUS?.toBoolean()) {
						helper.sendDiscordNotification(
							"Node backup job Failed for ${currentBuild.fullDisplayName}",
							"Job for ${jenkinsfileParams.agentName} agent has result of ${currentBuild.currentResult} in"
								+ " stage **${env.FAILED_STAGE_NAME}** with message: **${env.FAILURE_MESSAGE}**.",
							env.BUILD_URL,
							currentBuild.currentResult
						)
					}
				}
			}
		}
	}
}
