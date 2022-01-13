def call() {
    return new File(currentBuild.rawBuild.parent.definition.scriptPath).parent.toString()
}