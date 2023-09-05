# jenkins-shared-library
Shared libraries for the Jenkins pipeline jobs

## Shared pipeline

There is a ``defaultCiPipeline`` step that is the default pipeline for all jobs.  It will run the basic build, tests and linter 
steps. Each package/repo can customize its behavior like which OS or where to publish the artifacts.

````
defaultCiPipeline {
	operatingSystem = ['ubuntu']
	instanceSize = 'medium'
	publisher = 'docker'
	dockerImageName = 'symbolplatform/symbol-server-private'
	environment = 'python'
	otherEnvironments = ['python-ubuntu-base', 'python-ubuntu-latest', 'python-windows-lts']
	gitHubId = 'Symbol-Github-app'
}
````

``operatingSystem`` - is an array of OS the project needs to build on. ``ubuntu`` is the default.

``instanceSize`` - is the size of the instance to use for the build.  The options are ``small``, ``medium`` and ``xlarge``.  ``medium`` is the default.

``publisher`` - where to publish the artifacts.  The options are ``docker``, ``npm`` and ``pypi``.

``dockerImageName`` - The name of the docker image to publish

``gitHubId`` - The GitHub id use to authenticate

``environment`` - The environment to use for the build.  The options are ``python``, ``javascript``, ``cpp``, ``java`` and ``linter``.

``otherEnvironments`` - The other environments to build.

The environment and otherEnvironments are used to build the docker image name.
For example, if the environment is ``python`` then the docker image name will be ``symbolplatform/build-ci:python-ubuntu-22.04``.
