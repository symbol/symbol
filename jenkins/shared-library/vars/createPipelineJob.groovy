void call(Map jobConfiguration) {
	logger.logInfo("Pipeline job configuration: ${jobConfiguration}")

	jobDsl scriptText: """
		pipelineJob(jobName) {
			displayName(displayName)
			definition {
				cpsScm {
					lightweight(true)
					scm {
						git {
							branch('*/dev')
							remote {
								credentials(credentialsId)
								name('origin')
								url(repositoryUrl)
							}
						}
					}

					scriptPath(jenkinsfilePath)
				}
			}

			triggers {
				cron(trigger)
			}
		}
		""", additionalParameters: [
			jobName: jobConfiguration.jobName.toString(),
			repositoryUrl: jobConfiguration.repositoryUrl.toString(),
			credentialsId: jobConfiguration.credentialsId ? jobConfiguration.credentialsId.toString() : '',
			jenkinsfilePath: jobConfiguration.jenkinsfilePath.toString(),
			displayName: jobConfiguration.displayName.toString(),
			trigger: jobConfiguration.trigger ? jobConfiguration.trigger.toString() : ''
	]
}
