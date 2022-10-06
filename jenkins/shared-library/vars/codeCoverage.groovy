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
		'c8': { Integer target ->
			logCodeCoverageMinimum(target)
			runScript('npx c8@latest report')
			runScript("npx c8@latest check-coverage --lines ${target}")
		},
		'jacoco': { Integer target ->
			logger.logInfo("Minimum code coverage set in pom is ${readJacocoCoverageLimit()}")
			runScript('mvn jacoco:check@jacoco-check')
		}]

	codeCoverageCommand[tool](minimumCodeCoverage)
}

Integer readJacocoCoverageLimit() {
	Object rules = readMavenPom().build.plugins.find { plugin ->
		plugin.artifactId == 'jacoco-maven-plugin'
	}.executions.find { execution ->
		execution.id == 'jacoco-check'
	}.configuration
	Object configuration = new XmlSlurper().parseText(rules.toString())
	return Double.parseDouble(configuration.rules.rule.first().limits.limit.first().minimum.text()) * 100
}
