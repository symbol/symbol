plugins {
	`java-library`
	jacoco
	id("com.diffplug.spotless") version "6.25.0"
}

group = "org.symbol"
version = "3.3.1"

java {
	toolchain {
		languageVersion.set(JavaLanguageVersion.of(21))
	}
	withJavadocJar()
	withSourcesJar()
}

// The codebase is free of unchecked operations (generated setField uses checked asList coercion)
// and of deprecated-API usage; fail the build — in every project, including examples — if either
// creeps back in.
allprojects {
	tasks.withType<JavaCompile>().configureEach {
		options.compilerArgs.addAll(listOf("-Xlint:unchecked,deprecation", "-Werror"))
	}
}

repositories {
	mavenCentral()
}

dependencies {
	// Bouncy Castle is used only where the JDK does not provide a primitive directly:
	// Keccak (NEM hash), RIPEMD-160, HKDF-SHA256, and Ed25519 with a custom hasher.
	implementation("org.bouncycastle:bcprov-jdk18on:1.84")
	// JSON descriptor parsing (JsonDescriptor / facade createTransactionFromJson) and, in tests,
	// the catbuffer vector toJson() serializability checks.
	implementation("com.fasterxml.jackson.core:jackson-databind:2.17.1")

	testImplementation("org.junit.jupiter:junit-jupiter:5.10.2")
	testImplementation("org.hamcrest:hamcrest:2.2")
	testRuntimeOnly("org.junit.platform:junit-platform-launcher")
}

tasks.test {
	useJUnitPlatform {
		excludeTags("catvectors")
	}
	finalizedBy(tasks.jacocoTestReport)
	testLogging {
		events("passed", "skipped", "failed")
	}
}

// Shared discovery — both jacocoTestReport and jacocoTestCoverageVerification operate over the
// same merged set of .exec files (test + catVectors + examples).
val mergedJacocoExecs = fileTree(layout.buildDirectory).include("jacoco/*.exec")

tasks.jacocoTestReport {
	dependsOn(tasks.test)
	// `catVectors` writes its own .exec file into the merged set; declare ordering so Gradle's
	// strict validation doesn't flag jacocoTestReport as reading an undeclared input. The task
	// only runs when explicitly requested — mustRunAfter is a no-op when it isn't in the graph.
	mustRunAfter(tasks.named("catVectors"))
	// Pick up coverage from every JaCoCo-agent-instrumented run: the unit-test `test` task,
	// the cross-language `catVectors` Test task, plus any examples that ran (see
	// examples/build.gradle.kts where each JavaExec writes into this same jacoco directory).
	// Tasks that haven't run produce no .exec file — the merge silently degrades.
	executionData.setFrom(mergedJacocoExecs)
	reports {
		xml.required.set(true)
		html.required.set(true)
		xml.outputLocation.set(layout.buildDirectory.file("jacoco/test.xml"))
	}
	doLast {
		// Print an aggregate coverage summary to the console, plus a per-class breakdown for the
		// hand-written packages (the actionable surface). Generated packages are summarised
		// only — drill into them via the HTML report at build/reports/jacoco/test/html/.
		val xmlReport = reports.xml.outputLocation.get().asFile
		if (!xmlReport.exists()) return@doLast

		val counterRegex = Regex("""<counter\s+type="(\w+)"\s+missed="(\d+)"\s+covered="(\d+)"\s*/>""")
		val xmlText = xmlReport.readText()
		val byType = counterRegex.findAll(xmlText).groupBy { it.groupValues[1] }
		if (byType.isEmpty()) return@doLast

		println()
		println("Coverage (from ${xmlReport.relativeTo(rootDir)}):")
		for (type in listOf("INSTRUCTION", "BRANCH", "LINE", "COMPLEXITY", "METHOD", "CLASS")) {
			val last = byType[type]?.last() ?: continue
			val missed = last.groupValues[2].toInt()
			val covered = last.groupValues[3].toInt()
			val total = missed + covered
			val pct = if (total > 0) covered * 100.0 / total else 100.0
			println("  %-12s %7d / %7d  (%6.2f%%)".format(type, covered, total, pct))
		}

		// Per-class breakdown — parse the report DOM so we can pick out class-level INSTRUCTION
		// counters. Strip the DTD reference first so the parser doesn't try to fetch it.
		val cleanText = xmlText.replaceFirst(Regex("<!DOCTYPE[^>]*>"), "")
		val doc = javax.xml.parsers.DocumentBuilderFactory.newInstance()
				.newDocumentBuilder()
				.parse(cleanText.byteInputStream())
		val root = doc.documentElement

		// Generated catbuffer models / descriptors are emitted by the Python generator — surface
		// them as package-level totals only. The user-actionable surface is everything else.
		val generatedPkgs = setOf(
				"org/symbol/sdk/symbol/models",
				"org/symbol/sdk/symbol/descriptors",
				"org/symbol/sdk/nem/models",
				"org/symbol/sdk/nem/descriptors")

		data class ClassCov(val pkg: String, val name: String, val missed: Int, val covered: Int) {
			val total = missed + covered
			val pct = if (total > 0) covered * 100.0 / total else 100.0
		}

		val handWritten = mutableListOf<ClassCov>()
		val generatedPkgStats = mutableMapOf<String, Pair<Int, Int>>()

		fun directInstructionCounter(parent: org.w3c.dom.Element): Pair<Int, Int>? {
			val kids = parent.childNodes
			for (j in 0 until kids.length) {
				val node = kids.item(j)
				if (node is org.w3c.dom.Element && node.nodeName == "counter"
						&& node.getAttribute("type") == "INSTRUCTION") {
					return node.getAttribute("missed").toInt() to node.getAttribute("covered").toInt()
				}
			}
			return null
		}

		val packages = root.getElementsByTagName("package")
		for (i in 0 until packages.length) {
			val pkg = packages.item(i) as org.w3c.dom.Element
			val pkgName = pkg.getAttribute("name")
			if (pkgName in generatedPkgs) {
				directInstructionCounter(pkg)?.let { generatedPkgStats[pkgName] = it }
				continue
			}

			val classes = pkg.getElementsByTagName("class")
			for (j in 0 until classes.length) {
				val cls = classes.item(j) as org.w3c.dom.Element
				val (missed, covered) = directInstructionCounter(cls) ?: continue
				if (missed > 0) {
					val short = cls.getAttribute("name").substringAfterLast('/')
					handWritten.add(ClassCov(pkgName, short, missed, covered))
				}
			}
		}

		fun shortPkg(name: String): String =
				name.removePrefix("org/symbol/sdk").removePrefix("/").ifEmpty { "(root)" }

		if (handWritten.isNotEmpty()) {
			handWritten.sortBy { -it.missed }
			println()
			println("Hand-written classes with missing coverage (sorted by missed instructions):")
			println("  %-25s %-50s %6s   %s".format("PACKAGE", "CLASS", "MISS", "COV%"))
			for (c in handWritten) {
				println("  %-25s %-50s %6d  %6.2f%%".format(shortPkg(c.pkg), c.name, c.missed, c.pct))
			}
		}

		if (generatedPkgStats.isNotEmpty()) {
			println()
			println("Generated packages (per-class detail in the HTML report):")
			for ((pkg, mc) in generatedPkgStats.entries.sortedByDescending { it.value.first }) {
				val (missed, covered) = mc
				val total = missed + covered
				val pct = if (total > 0) covered * 100.0 / total else 100.0
				println("  %-25s miss=%6d  cov=%6.2f%%".format(shortPkg(pkg), missed, pct))
			}
		}

		val htmlReport = reports.html.outputLocation.get().asFile.resolve("index.html")
		println()
		println("HTML report (per-class / per-method / per-line drill-down):")
		println("  ${htmlReport.relativeTo(rootDir)}")
		println()
	}
}

tasks.jacocoTestCoverageVerification {
	dependsOn(tasks.test)
	// Same ordering constraint as jacocoTestReport — catVectors's .exec contributes when it
	// runs, but isn't a hard prerequisite for `check`.
	mustRunAfter(tasks.named("catVectors"))
	executionData.setFrom(mergedJacocoExecs)
	violationRules {
		rule {
			limit {
				counter = "INSTRUCTION"
				minimum = "0.90".toBigDecimal()
			}
		}
		rule {
			limit {
				counter = "CLASS"
				minimum = "0.95".toBigDecimal()
			}
		}
	}
}

// Wire coverage verification into `check` so a regression below the 90% bar fails the build.
tasks.check {
	dependsOn(tasks.jacocoTestCoverageVerification)
}

tasks.javadoc {
	(options as StandardJavadocDocletOptions).addStringOption("Xdoclint:none", "-quiet")
}

spotless {
	java {
		target("src/**/*.java")

		eclipse().configFile("eclipse-formatter.xml")
		trimTrailingWhitespace()
		endWithNewline()
		removeUnusedImports()
		importOrder("java", "javax", "org.bouncycastle", "", "org.symbol")
	}
}

// Vectors: cross-language test vectors live in <repo>/tests/vectors/{nem,symbol}; mirrors `npm run vectors`.
val vectors by tasks.registering(JavaExec::class) {
	group = "verification"
	description = "Run cross-language test vectors against the Java SDK."
	classpath = sourceSets["test"].runtimeClasspath + sourceSets["main"].runtimeClasspath
	mainClass.set("org.symbol.sdk.vectors.AllVectors")
	val blockchain = providers.environmentVariable("BLOCKCHAIN").orElse("symbol").get()
	val gitRoot = rootProject.layout.projectDirectory.asFile.parentFile.parentFile
	args = listOf("--vectors", "${gitRoot}/tests/vectors/${blockchain}/crypto", "--blockchain", blockchain)
}

val catVectors by tasks.registering(Test::class) {
	group = "verification"
	description = "Run catbuffer model vectors (mirrors `npm run catvectors`)."
	useJUnitPlatform {
		includeTags("catvectors")
	}
	testClassesDirs = sourceSets["test"].output.classesDirs
	classpath = sourceSets["test"].runtimeClasspath
	// Point CatbufferVectorsHelper at <repo>/tests/vectors so the vectors resolve without
	// the caller needing to export SCHEMAS_PATH. An explicit environment override still wins
	// if set externally, mirroring the `npm run catvectors` behavior in the JS SDK.
	val gitRoot = rootProject.layout.projectDirectory.asFile.parentFile.parentFile
	environment("SCHEMAS_PATH", providers.environmentVariable("SCHEMAS_PATH").orElse("${gitRoot}/tests/vectors").get())
	testLogging {
		events("passed", "skipped", "failed")
	}
}

// Regenerate the catbuffer-derived Java model classes under
// org.symbol.sdk.{nem,symbol} by shelling out to scripts/run_catbuffer_generator.sh.
// Hand-written files (Address, KeyPair, Network, ...) are preserved by the script,
// which only deletes files carrying the "Auto-generated by sdk/java/generator" header.
val generateModels by tasks.registering(Exec::class) {
	group = "build"
	description = "Regenerate Java catbuffer model classes from catbuffer/schemas."
	workingDir = layout.projectDirectory.asFile
	// The script formats its own output by shelling to gradlew; suppress that here (we are already
	// inside a Gradle build) and run spotlessApply as a finalizer instead, avoiding nested Gradle.
	environment("SKIP_SPOTLESS", "1")
	commandLine("bash", "scripts/run_catbuffer_generator.sh")
	finalizedBy("spotlessApply")
}

// Regenerate the catbuffer-derived typed descriptor classes under
// org.symbol.sdk.{nem,symbol}.descriptors by shelling out to
// scripts/run_catbuffer_descriptor_generator.sh.
val generateDescriptors by tasks.registering(Exec::class) {
	group = "build"
	description = "Regenerate Java catbuffer typed descriptor classes from catbuffer/schemas."
	workingDir = layout.projectDirectory.asFile
	// The script formats its own output by shelling to gradlew; suppress that here (we are already
	// inside a Gradle build) and run spotlessApply as a finalizer instead, avoiding nested Gradle.
	environment("SKIP_SPOTLESS", "1")
	commandLine("bash", "scripts/run_catbuffer_descriptor_generator.sh")
	finalizedBy("spotlessApply")
}
