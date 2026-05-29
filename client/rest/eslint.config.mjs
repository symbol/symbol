import sharedDefaultConfig from '../../linters/javascript/default.mjs';
import sharedTestConfig from '../../linters/javascript/test.mjs';
import { defineConfig } from 'eslint/config';
import importPlugin from 'eslint-plugin-import-x';
import jsdoc from 'eslint-plugin-jsdoc';
import globals from 'globals';

export default defineConfig([
	{
		ignores: ['src/plugins/rosetta/openApi/**']
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
				...globals.es2024,
				...globals.node
			}
		}
	},
	sharedDefaultConfig,
	{
		rules: {
			'import/extensions': ['error', 'ignorePackages'],
			'no-underscore-dangle': ['error', {
				allow: ['_id']
			}]
		}
	},
	{
		files: ['test/**/*.js'],
		languageOptions: {
			globals: {
				...globals.mocha
			}
		},
		...sharedTestConfig,
		rules: {
			...sharedTestConfig.rules,
			'no-underscore-dangle': ['error', {
				allow: ['_id', 'high_', 'low_']
			}]
		}
	}
]);

