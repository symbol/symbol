void call(Object config) {
	verifyCodeCoverageResult(config.codeCoverageTool, config.minimumCodeCoverage)
	uploadCodeCoverage(config.packageId)
}

void uploadCodeCoverage(String flag) {
	String repoName = env.GIT_URL.tokenize('/').last().split('\\.')[0].toUpperCase()
	withCredentials([string(credentialsId: "${repoName}_CODECOV_ID", variable: 'CODECOV_TOKEN')]) {
		logger.logInfo("Uploading code coverage for ${flag}")
		runScript("codecov --verbose --nonZero --rootDir ${env.WORKSPACE} --flags ${flag} --dir .")
	}
}

void logCodeCoverageMinimum(Integer minimumCodeCoverage) {
	logger.logInfo("Minimum code coverage is ${minimumCodeCoverage}")
}

void verifyCodeCoverageResult(String tool, Integer minimumCodeCoverage) {
	Map codeCoverageCommand = [
		'coverage': { Integer target ->
			logCodeCoverageMinimum(target)
			runScript('coverage xml')
			runScript("coverage report --fail-under=${target}")
		},
		'nyc': { Integer target ->
			logCodeCoverageMinimum(target)
			runScript('npx nyc@latest report --lines')
			runScript("npx nyc@latest check-coverage --lines ${target}")
		},
		'jacoco': { Integer target ->
			logger.logInfo('Minimum code coverage is set pom.xml')
			runScript('mvn jacoco:check@jacoco-check')
		}]

	codeCoverageCommand[tool](minimumCodeCoverage)
}
