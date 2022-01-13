# jenkins-shared-library
Shared libraries for the Jenkins pipeline jobs

## Shared pipeline

There is a ``default_ci_pipeline`` step in which is the default pipeline for all jobs.  It will run the basic build, tests and linter 
steps. Each package/repo can customize its behavior like which OS or where to publish the artifacts.

````
default_ci_pipeline {
    platform = ['ubuntu']
    docker_image_name = 'symbolplatform/symbol-server-private'
}
````

``platform`` - is an array of OS the package needs to build on.

``docker_image_name`` - The name of the docker image to publish

``npm`` - set to true to publish npm packages
