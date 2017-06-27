/** @module utils/future */

export default {
	/**
	 * Makes a future retryable.
	 * @param {Function} futureSupplier The function that returns a new instance of the wrapped future.
	 * @param {Numeric} maxAttempts The maximum number of attempts.
	 * @param {Function} waitTimeSupplier The function that calculates the amount of time to wait after a failure.
	 * @returns {Promise} A promise that is resolved when the future succeeds or the maximum number of attempts have failed.
	 */
	makeRetryable: (futureSupplier, maxAttempts, waitTimeSupplier) => {
		let numRemainingAttempts = maxAttempts;
		const step = (resolve, reject) => {
			--numRemainingAttempts;
			futureSupplier()
				.then(o => {
					resolve(o);
				})
				.catch(err => {
					if (0 === numRemainingAttempts) {
						reject(err);
						return;
					}

					const waitTime = waitTimeSupplier(maxAttempts - numRemainingAttempts, err);
					setTimeout(() => { step(resolve, reject); }, waitTime);
				});
		};

		return new Promise((resolve, reject) => {
			step(resolve, reject);
		});
	}
};
