// Create a folder in Jenkins.
void call(String folder, String displayName) {
	jobDsl scriptText: '''
			folder(folder) {
				displayName(displayName)
				}
		''', additionalParameters: [
			folder: folder,
			displayName: displayName
		]
}
