import sharedDefaultConfig from '../../linters/javascript/default.mjs';
import sharedTestConfig from '../../linters/javascript/test.mjs';
import js from '@eslint/js';
import { defineConfig } from 'eslint/config';
import importPlugin from 'eslint-plugin-import-x';
import jsdoc from 'eslint-plugin-jsdoc';
import globals from 'globals';
import tseslint from 'typescript-eslint';

export default defineConfig([
	{
		ignores: ['_build/**', 'dist/**', 'ts/**/*.d.ts']
	},
	{
		plugins: {
			import: importPlugin,
			jsdoc
		},
		languageOptions: {
			globals: {
				...globals.es2024,
				...globals.nodeBuiltin
			}
		}
	},

	js.configs.recommended,
	tseslint.configs.strict,
	jsdoc.configs['flat/recommended-error'],

	sharedDefaultConfig,
	{
		rules: {
			'import/extensions': ['error', 'ignorePackages'],
			'@typescript-eslint/no-extraneous-class': 'off'
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
		files: ['vectors/**/*.js'],
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
	},
	{
		files: ['examples/**/*.js'],
		...sharedTestConfig,
		rules: {
			...sharedTestConfig.rules,
			'no-console': 'off'
		}
	}
]);
