/**
 * Rosetta
 * Build Once. Integrate Your Blockchain Everywhere.
 *
 * The version of the OpenAPI document: 1.4.13
 * 
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 *
 */

import ApiClient from '../ApiClient.js';
import SigningPayload from './SigningPayload.js';

/**
 * The ConstructionPayloadsResponse model module.
 * @module model/ConstructionPayloadsResponse
 * @version 1.4.13
 */
class ConstructionPayloadsResponse {
	/**
	 * Constructs a new <code>ConstructionPayloadsResponse</code>.
	 * ConstructionTransactionResponse is returned by &#x60;/construction/payloads&#x60;. It contains an unsigned transaction blob (that is usually needed to construct the a network transaction from a collection of signatures) and an array of payloads that must be signed by the caller.
	 * @alias module:model/ConstructionPayloadsResponse
	 * @param unsignedTransaction {String}
	 * @param payloads {Array.<module:model/SigningPayload>}
	 */
	constructor(unsignedTransaction, payloads) {

		ConstructionPayloadsResponse.initialize(this, unsignedTransaction, payloads);
	}

	/**
	 * Initializes the fields of this object.
	 * This method is used by the constructors of any subclasses, in order to implement multiple inheritance (mix-ins).
	 * Only for internal use.
	 */
	static initialize(obj, unsignedTransaction, payloads) {
		obj['unsigned_transaction'] = unsignedTransaction;
		obj['payloads'] = payloads;
	}

	/**
	 * Constructs a <code>ConstructionPayloadsResponse</code> from a plain JavaScript object, optionally creating a new instance.
	 * Copies all relevant properties from <code>data</code> to <code>obj</code> if supplied or a new instance if not.
	 * @param {Object} data The plain JavaScript object bearing properties of interest.
	 * @param {module:model/ConstructionPayloadsResponse} obj Optional instance to populate.
	 * @return {module:model/ConstructionPayloadsResponse} The populated <code>ConstructionPayloadsResponse</code> instance.
	 */
	static constructFromObject(data, obj) {
		if (data) {
			obj = obj || new ConstructionPayloadsResponse();

			if (data.hasOwnProperty('unsigned_transaction')) {
				obj['unsigned_transaction'] = ApiClient.convertToType(data['unsigned_transaction'], 'String');
			}
			if (data.hasOwnProperty('payloads')) {
				obj['payloads'] = ApiClient.convertToType(data['payloads'], [SigningPayload]);
			}
		}
		return obj;
	}

	/**
	 * Validates the JSON data with respect to <code>ConstructionPayloadsResponse</code>.
	 * @param {Object} data The plain JavaScript object bearing properties of interest.
	 * @return {boolean} to indicate whether the JSON data is valid with respect to <code>ConstructionPayloadsResponse</code>.
	 */
	static validateJSON(data) {
		// check to make sure all required properties are present in the JSON string
		for (const property of ConstructionPayloadsResponse.RequiredProperties) {
			if (!data.hasOwnProperty(property)) {
				throw new Error("The required field `" + property + "` is not found in the JSON data: " + JSON.stringify(data));
			}
		}
		// ensure the json data is a string
		if (data['unsigned_transaction'] && !(typeof data['unsigned_transaction'] === 'string' || data['unsigned_transaction'] instanceof String)) {
			throw new Error("Expected the field `unsigned_transaction` to be a primitive type in the JSON string but got " + data['unsigned_transaction']);
		}
		if (data['payloads']) { // data not null
			// ensure the json data is an array
			if (!Array.isArray(data['payloads'])) {
				throw new Error("Expected the field `payloads` to be an array in the JSON data but got " + data['payloads']);
			}
			// validate the optional field `payloads` (array)
			for (const item of data['payloads']) {
				SigningPayload.validateJSON(item);
			};
		}

		return true;
	}


}

ConstructionPayloadsResponse.RequiredProperties = ["unsigned_transaction", "payloads"];

/**
 * @member {String} unsigned_transaction
 */
ConstructionPayloadsResponse.prototype['unsigned_transaction'] = undefined;

/**
 * @member {Array.<module:model/SigningPayload>} payloads
 */
ConstructionPayloadsResponse.prototype['payloads'] = undefined;






export default ConstructionPayloadsResponse;
