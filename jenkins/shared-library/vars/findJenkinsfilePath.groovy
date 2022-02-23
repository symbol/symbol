import java.nio.file.Paths

String call() {
	String scriptPath = currentBuild.rawBuild.parent.definition.scriptPath
	if (scriptPath.indexOf('/') == -1) {
		// Jenkinsfile is at the project's root
		return '.'
	}
	return Paths.get(scriptPath).parent.toString()
}
