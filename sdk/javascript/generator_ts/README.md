# javascript catbuffer transaction descriptor generator

## Generate catbuffer

```sh
./scripts/run_catbuffer_generator_ts.sh
```

Run test vectors (assuming node project is `sdk/javascript`):

```bash
SCHEMAS_PATH="$(git rev-parse --show-toplevel)/tests/vectors" npm run catvectors
```
