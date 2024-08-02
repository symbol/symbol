void call(Object config) {
	verifyCodeCoverageResult(config.codeCoverageTool, config.minimumCodeCoverage)
	uploadCodeCoverage(config.packageId)
}

void uploadCodeCoverage(String flag) {
	final String repositoryName = helper.resolveRepositoryName()
	withCredentials([string(credentialsId: "${repositoryName.toUpperCase()}_CODECOV_ID", variable: 'CODECOV_TOKEN')]) {
		final String ownerName = helper.resolveOrganizationName()
		final Boolean isPublicRepo = githubHelper.isGitHubRepositoryPublic(ownerName, repositoryName)
		String codeCoverageCommand = "codecov --verbose --flags ${flag} --dir ."

		if (isPublicRepo) {
			codeCoverageCommand += ' --nonZero'
		}

		logger.logInfo("Uploading code coverage for ${flag}")
		runScript(codeCoverageCommand)
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
		},
		'golang': { Integer target ->
			logCodeCoverageMinimum(target)
			runScript('go tool cover -func coverage.out')
			String coverage = runScript("go tool cover -func coverage.out | grep total | awk '{print substr(\$3, 1, length(\$3)-1)}'", true)
			if (coverage.toInteger() < target) {
				throw new RuntimeException("Code coverage is below the minimum threshold of ${target}")
			}
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
