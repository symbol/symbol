void call(Object config) {
	verifyCodeCoverageResult(config.codeCoverageTool, config.minimumCodeCoverage)
	uploadCodeCoverage(config.packageId)
}

void uploadCodeCoverage(String flag) {
	String repoName = env.GIT_URL.tokenize('/').last().split('\\.')[0].toUpperCase()
	withCredentials([string(credentialsId: "${repoName}_CODECOV_ID", variable: 'CODECOV_TOKEN')]) {
		logger.logInfo("Uploading code coverage for ${flag}")
		runScript("codecov --required --root ${env.WORKSPACE} --flags ${flag} .")
	}
}

void verifyCodeCoverageResult(String tool, Integer minimumCodeCoverage) {
	Map codeCoverageCommand = [
		'coverage': { target ->
			runScript('coverage xml')
			runScript("coverage report --fail-under=${target}")
		},
		'nyc': { target ->
			runScript('npx nyc@latest report --lines')
			runScript("npx nyc@latest check-coverage --lines ${target}")
		}]

	logger.logInfo("Minimum code coverage is ${minimumCodeCoverage}")
	codeCoverageCommand[tool](minimumCodeCoverage)
}
