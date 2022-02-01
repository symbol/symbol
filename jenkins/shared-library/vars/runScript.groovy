import java.nio.file.Paths

void call(String scriptPath, String filename) {
	String scriptFilepath = Paths.get(scriptPath, filename)
	call(scriptFilepath)
}

void call(String scriptFilepath) {
	call(scriptFilepath, scriptFilepath, false)
}

void call(String scriptFilepath, String label='', Boolean returnStdout, Boolean returnStatus=false, String encoding='') {
	logger.logInfo("Running script ${scriptFilepath}")
	if (isUnix()) {
		sh label: label, script: scriptFilepath, encoding: encoding, returnStdout: returnStdout, returnStatus: returnStatus
	}
	else {
		bat label: label, script: scriptFilepath, encoding: encoding, returnStdout: returnStdout, returnStatus: returnStatus
	}
}
