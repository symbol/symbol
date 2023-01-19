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

								relativeTargetDirectory {
									// Specify a local directory (relative to the workspace root) where the Git repository will be checked out.
									relativeTargetDir(targetDirectory)
								}
							}
						}
					}

					scriptPath(jenkinsfilePath)
				}
			}

			logRotator {
				// If specified, only up to this number of builds have their artifacts retained.
				artifactNumToKeep(buildsToKeep)

				// If specified, only up to this number of build records are kept.
				numToKeep(buildsToKeep)
			}

			properties {
				pipelineTriggers {
					triggers {
						cron {
							spec(schedule)
						}
					}
				}
			}
		}
		""", additionalParameters: [
			jobName: jobConfiguration.jobName.toString(),
			ownerAndProject: jobConfiguration.ownerAndProject.toString(),
			credentialsId: jobConfiguration.credentialsId ? jobConfiguration.credentialsId.toString() : '',
			jenkinsfilePath: jobConfiguration.jenkinsfilePath.toString(),
			displayName: jobConfiguration.displayName.toString(),
			schedule: jobConfiguration.cronTrigger ? jobConfiguration.cronTrigger.toString() : '',
			buildsToKeep: jobConfiguration.buildsToKeep ?: 14,
			targetDirectory: jobConfiguration.targetDirectory ? jobConfiguration.targetDirectory.toString() : '',
	]
}
