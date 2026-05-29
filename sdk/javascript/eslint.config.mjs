import sharedDefaultConfig from '../../linters/javascript/default.mjs';
import sharedTestConfig from '../../linters/javascript/test.mjs';
import { defineConfig } from 'eslint/config';
import importPlugin from 'eslint-plugin-import-x';
import jsdoc from 'eslint-plugin-jsdoc';
import globals from 'globals';

export default defineConfig([
	{
		ignores: ['_build/**', 'dist/**']
	},
	{
		linterOptions: {
			reportUnusedDisableDirectives: false
		}
	},
	{
		plugins: {
			import: importPlugin,
			jsdoc
		},
		languageOptions: {
			globals: {
				...globals.es2024
			}
		}
	},
	sharedDefaultConfig,
	{
		rules: {
			'import/extensions': ['error', 'ignorePackages']
		}
	},
	{
		files: ['test/**/*.js'],
		languageOptions: {
			globals: {
				...globals.mocha
			}
		},
		...sharedTestConfig
	},
	{
		files: ['examples/**/*.js', 'vectors/**/*.js'],
		languageOptions: {
			globals: {
				...globals.mocha
			}
		},
		...sharedTestConfig,
		rules: {
			...sharedTestConfig.rules,
			'no-console': 'off'
		}
	}
]);

