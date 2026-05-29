export default {
	languageOptions: {
		ecmaVersion: 2022,
		sourceType: 'module'
	},
	rules: {
		'function-paren-newline': ['error', 'multiline'],
		'function-call-argument-newline': ['off'],
		'no-console': [2],
		indent: ['error', 'tab'],
		'linebreak-style': ['error', 'unix'],
		quotes: ['error', 'single'],
		semi: ['error', 'always'],
		yoda: ['error', 'always'],
		curly: ['error', 'multi-or-nest', 'consistent'],
		'max-len': ['error', {
			code: 140
		}],
		'max-classes-per-file': ['off'],
		'prefer-object-spread': ['off'],
		'nonblock-statement-body-position': ['error', 'below'],
		'implicit-arrow-linebreak': ['off'],
		'no-tabs': ['off'],
		'no-bitwise': ['off'],
		'no-plusplus': ['off'],
		'no-mixed-operators': ['error', {
			allowSamePrecedence: true
		}],
		'no-param-reassign': ['error', {
			props: false
		}],
		'no-underscore-dangle': ['error', {
			allowAfterThis: true
		}],
		camelcase: ['off'],
		'comma-dangle': ['error', 'never'],
		'default-case': ['off'],
		'arrow-parens': ['error', 'as-needed'],
		'func-names': ['error', 'never'],
		'func-style': ['error', 'expression'],
		'wrap-iife': ['error', 'inside'],
		'prefer-destructuring': ['error', {
			object: true,
			array: false
		}],
		'jsdoc/reject-any-type': ['off'],
		'jsdoc/reject-function-type': ['off'],
		'import/order': ['error', {
			'newlines-between': 'never',
			groups: ['index', 'sibling', 'parent', 'internal', 'external', 'builtin'],
			alphabetize: {
				order: 'asc'
			}
		}],
		'sort-imports': ['error', {
			ignoreDeclarationSort: true
		}],
		'import/extensions': ['error', 'never'],
		'import/no-absolute-path': ['error'],
		'import/no-unresolved': [2, {
			ignore: ['^symbol-sdk/']
		}],
		'import/no-deprecated': ['error'],
		'import/named': ['error']
	}
};
