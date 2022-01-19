import java.nio.file.Paths

String call() {
	return Paths.get(currentBuild.rawBuild.parent.definition.scriptPath).parent.toString()
}
