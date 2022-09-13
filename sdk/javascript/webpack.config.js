// Generated using webpack-cli https://github.com/webpack/webpack-cli

const WasmPackPlugin = require('@wasm-tool/wasm-pack-plugin');
const webpack = require('webpack');
const path = require('path');

const isProduction = 'production' === process.env.NODE_ENV;

const config = {
	entry: './src/cdn.js',
	output: {
		filename: '../index.js'
	},
	plugins: [
		// Add your plugins here
		// Learn more about plugins from https://webpack.js.org/configuration/plugins/
		new webpack.ProvidePlugin({
			process: 'process/browser',
			Buffer: ['buffer', 'Buffer']
		}),
		new WasmPackPlugin({
			crateDirectory: 'wasm'
		})
	],
	module: {
		rules: [
			{
				test: /\.(js|jsx)$/i,
				loader: 'babel-loader'
			},
			{
				test: /\.(eot|svg|ttf|woff|woff2|png|jpg|gif)$/i,
				type: 'asset'
			},
			{
				test: /\.wasm$/,
				type: 'webassembly/async'
			}

			// Add your rules for custom modules here
			// Learn more about loaders from https://webpack.js.org/loaders/
		]
	},
	resolve: {
		extensions: ['.ts', '.js'],
		fallback: {
			crypto: require.resolve('crypto-browserify'),
			fs: false,
			path: require.resolve('path-browserify'),
			stream: require.resolve('stream-browserify'),
			url: require.resolve('url'),
			util: require.resolve('util')
		}
	},
	experiments: {
		asyncWebAssembly: true
	}
};

module.exports = () => {
	if (isProduction)
		config.mode = 'production';
	else
		config.mode = 'development';
	return config;
};
