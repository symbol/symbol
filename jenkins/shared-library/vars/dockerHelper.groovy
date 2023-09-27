void updateDockerImage(String targetImageName, String sourceImageName, String architecture) {
	lock(targetImageName) {
		if (!isDockerImageInRegistry(targetImageName)) {
			pushImage(sourceImageName, targetImageName)
			return
		}

		// The docker buildx append command will always append to an existing image
		// and doesnt override if the architecture manifest already exist.
		// Need to pull the image manifest to check if the architecture already exist.
		String imageManifest = resolveDockerImageManifestInRegistry(targetImageName)
		Object packageJson = readJSON text: imageManifest
		if (isSingleDigestImage(packageJson)) {
			runScript("docker pull ${targetImageName}")
			String localManifest = runScript("docker image inspect ${targetImageName}", true)
			Object localPackageJson = readJSON text: localManifest
			println("Local architecture: ${localPackageJson[0].Architecture}")
			if (architecture == localPackageJson[0].Architecture) {
				// same architecture so overwrite
				pushImage(sourceImageName, targetImageName)
			} else {
				// different architecture so append
				runScript("docker buildx imagetools create --tag ${targetImageName} --append ${sourceImageName}")
			}
		}
		else {
			// This is a multi-architecture image
			List<String> imageDigests = resolveDockerImageDigests(packageJson, sourceImageName, targetImageName, architecture)
			String digestList = imageDigests.join(' ')
			print("Digest list: ${digestList}")
			runScript("docker buildx imagetools create --tag ${targetImageName} ${digestList}")
		}

		// log the manifest for the updated image
		runScript("docker buildx imagetools inspect ${targetImageName}")
	}
}

void pushImage(String sourceImageName, String targetImageName) {
	runScript("docker tag ${sourceImageName} ${targetImageName}")
	runScript("docker push ${targetImageName}")
}

boolean isDockerImageInRegistry(String imageName) {
	return runScript("docker buildx imagetools inspect ${imageName}", false, true) == 0
}

String resolveDockerImageManifestInRegistry(String imageName) {
	return runScript("docker buildx imagetools inspect ${imageName} --raw", true)
}

boolean isSingleDigestImage(Object packageJson) {
	return null == packageJson.manifests
}

List<String> resolveDockerImageDigests(Object packageJson, String latestImageName, String destImageName, String architecture) {
	Map<String, String> digests = [:]

	digests.put(architecture, latestImageName)
	packageJson.manifests.each { manifest ->
		print "Manifest: ${manifest.platform.architecture}, ${manifest.digest}"
		if (!digests.containsKey(manifest.platform.architecture)) {
			digests.put(manifest.platform.architecture, "${destImageName}@${manifest.digest}")
		}
	}

	return digests.values().toList()
}

void dockerBuildAndPushImage(String imageName, String buildArgs='.') {
	runScript("docker build -t ${imageName} ${buildArgs}")
	runScript("docker push ${imageName}")
}

void loginAndRunCommand(String dockerCredentialsId, String hostName, Closure command) {
	withCredentials([usernamePassword(credentialsId: dockerCredentialsId,
			usernameVariable: 'DOCKER_ID',
			passwordVariable: 'DOCKER_PASSWORD')]) {
		withEnv(["DOCKER_REGISTRY_HOSTNAME=${hostName}"]) {
			runScript('echo $DOCKER_PASSWORD | docker login $DOCKER_REGISTRY_HOSTNAME -u $DOCKER_ID --password-stdin')
			command()
		}
	}
}

void tagDockerImage(String operatingSystem, String dockerUrl, String dockerCredentialsId, String imageName, String destImageName) {
	// Windows container does not support docker buildx so just push the image
	if ('windows' == operatingSystem) {
		docker.withRegistry(dockerUrl, dockerCredentialsId) {
			String tag = destImageName.split(':')[1]
			docker.image(imageName).push(tag)
		}
	} else {
		loginAndRunCommand(dockerCredentialsId, dockerUrl) {
			final String hostName = helper.resolveUrlHostName(dockerUrl)
			updateDockerImage("${hostName}/${destImageName}", "${hostName}/${imageName}", "${ARCHITECTURE}")
		}
	}
}
