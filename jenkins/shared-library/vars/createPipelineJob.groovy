void call(Map jobConfiguration) {
	logger.logInfo("Pipeline job configuration: ${jobConfiguration}")

	jobDsl scriptText: """
		pipelineJob(jobName) {
			displayName(displayName)
			definition {
				cpsScm {
					lightweight(false)
					scm {
						git {
							branch('*/dev')
							remote {
								github(ownerAndProject, 'https')
								credentials(credentialsId)
							}

							extensions {
								// Delete the contents of the workspace before building, ensuring a fully fresh workspace.
								wipeWorkspace()
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
			ownerAndProject: jobConfiguration.ownerAndProject.toString(),
			credentialsId: jobConfiguration.credentialsId ? jobConfiguration.credentialsId.toString() : '',
			jenkinsfilePath: jobConfiguration.jenkinsfilePath.toString(),
			displayName: jobConfiguration.displayName.toString(),
			trigger: jobConfiguration.trigger ? jobConfiguration.trigger.toString() : ''
	]
}
