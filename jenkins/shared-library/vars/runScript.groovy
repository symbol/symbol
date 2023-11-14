import java.nio.file.Paths

void call(String scriptPath, String filename) {
	String scriptFilepath = Paths.get(scriptPath, filename)
	call(scriptFilepath)
}

void call(String scriptFilepath) {
	call(scriptFilepath, scriptFilepath, false)
}

void call(String scriptFilepath, boolean returnStdout, boolean returnStatus) {
	call(scriptFilepath, scriptFilepath, returnStdout, returnStatus)
}

void call(String scriptFilepath, String label='', Boolean returnStdout, Boolean returnStatus=false, String encoding='') {
	logger.logInfo("Running script `${scriptFilepath}`")
	if (isUnix()) {
		sh label: label, script: scriptFilepath, encoding: encoding, returnStdout: returnStdout, returnStatus: returnStatus
	}
	else {
		bat label: label, script: scriptFilepath, encoding: encoding, returnStdout: returnStdout, returnStatus: returnStatus
	}
}

void withBash(String scriptFilepath) {
	if (isUnix()) {
		call("bash -c '${scriptFilepath}'", scriptFilepath, false)
	} else {
		// Force the login scripts to run on Windows so that the PATH is set correctly
		// from the user's profile.
		call("bash --login -c '${scriptFilepath}'", scriptFilepath, false)
	}
}
