// Generated using webpack-cli https://github.com/webpack/webpack-cli

const webpack = require('webpack');
// const path = require('path');

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
			}

			// Add your rules for custom modules here
			// Learn more about loaders from https://webpack.js.org/loaders/
		]
	},
	resolve: {
		extensions: ['.ts', '.js'],
		fallback: {
			crypto: require.resolve('crypto-browserify'),
			stream: require.resolve('stream-browserify'),
			url: require.resolve('url')
		}
	}
};

module.exports = () => {
	if (isProduction)
		config.mode = 'production';
	else
		config.mode = 'development';
	return config;
};
