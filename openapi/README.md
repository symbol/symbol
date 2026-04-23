# symbol-openapi

OpenAPI specification for catapult-rest.

This project lives in the [Symbol monorepo](https://github.com/symbol/symbol) under `openapi/`.

## Requirements

* Node.js 20 LTS or higher

YAML style and syntax for files under `spec/` are checked by the monorepo **Linters** job (`linters/scripts/lint_yaml.sh` -> `yamllint` with `linters/yaml/.yamllint`). Run that from the repository root if you need the same check locally:

```sh
./linters/scripts/lint_yaml.sh
```

## Installation

1. Clone the monorepo and enter this package.

```
git clone https://github.com/symbol/symbol.git
cd symbol/openapi
```

2. Install dependencies.

```
npm install
```

## Commands

### Build

Compile the specification. The generated output is saved under the `_build` directory.

```
npm run build
```

### Test

Checks links in the built specification (`_build/openapi3.yml`).

```
npm run test
```

In CI the flow is split into dedicated stages:

- `scripts/ci/lint.sh` -> `npm run lint` (OpenAPI lint on `spec/openapi.yml`)
- `scripts/ci/build.sh` -> `npm run build` (bundle to `_build/openapi3.yml`)
- `scripts/ci/test.sh` -> `npm run test`

### Postman

Generate a Postman collection from the built specification.

```
npm run postman
```

### Release (monorepo)

Release in monorepo is component-scoped (`openapi/`) and produces versioned OpenAPI + Postman artifacts.

1. Bump `version` in `package.json` and update `CHANGELOG.md`.
2. Run publish script:

```
scripts/ci/publish.sh
```

This prepares:

- `_build/openapi3.yml`
- `_build/openapi3.json`
- `_build/postman.json`
- `_build/v<version>/openapi3.yml`
- `_build/v<version>/openapi3.json`
- `_build/v<version>/postman.json`

Optional: create a GitHub release with assets (requires authenticated `gh` CLI):

```sh
OPENAPI_RELEASE_CREATE_GH=1 OPENAPI_RELEASE_TAG=openapi/v<version> scripts/ci/publish.sh
```

## Contributing

Before contributing please [read this](CONTRIBUTING.md).

## License

Licensed under the [Apache License 2.0](LICENSE).
