import java.nio.file.Paths

String getTempPathInWorkspace(String filepath) {
	return Paths.get("${WORKSPACE_TMP}", filepath).toString()
}

String copyToTempFile(String data, String destinationFilepath) {
	return copyToLocalFile(data, getTempPathInWorkspace(destinationFilepath))
}

String copyToLocalFile(String data, String destinationFilepath) {
	String parentPath = Paths.get(destinationFilepath).parent
	dir(parentPath) {
		writeFile(file: destinationFilepath, text: data)
		logger.logInfo("Wrote data to ${destinationFilepath}")
	}

	return destinationFilepath
}

Boolean isEqual(String currentBranch, String releaseBranch) {
	return currentBranch == releaseBranch
}
