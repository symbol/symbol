import js from '@eslint/js';
import { defineConfig } from 'eslint/config';
import importPlugin from 'eslint-plugin-import-x';
import jsdoc from 'eslint-plugin-jsdoc';

import defaultConfig from '../linters/javascript/default.mjs';

export default defineConfig([
	js.configs.recommended,
	jsdoc.configs['flat/recommended-error'],
	{
		...defaultConfig,
		plugins: {
			...defaultConfig.plugins,
			import: importPlugin,
			jsdoc
		},
		languageOptions: {
			...defaultConfig.languageOptions,
			globals: {
				...defaultConfig.languageOptions?.globals,
				Buffer: 'readonly',
				TextDecoder: 'readonly',
				TextEncoder: 'readonly',
				WebSocket: 'readonly',
				console: 'readonly',
				fetch: 'readonly',
				process: 'readonly',
				setTimeout: 'readonly'
			}
		}
	},
	{
		rules: {
			'import/extensions': ['error', 'ignorePackages'],
			'no-console': 'off',
			'max-len': ['error', {
				code: 88,
				ignoreTrailingComments: true
			}],
			'function-paren-newline': ['off'],
			'prefer-destructuring': ['off'],
			'no-restricted-syntax': ['error', 'ForInStatement'],
			'jsdoc/require-jsdoc': ['off'],
			'operator-linebreak': ['error', 'after'],
			'func-style': ['error', 'declaration', {
				allowArrowFunctions: true
			}],
			'no-await-in-loop': ['off'],
			'import/no-extraneous-dependencies': ['off']
		}
	}
]);
