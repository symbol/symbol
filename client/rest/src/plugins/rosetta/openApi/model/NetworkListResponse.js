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
import NetworkIdentifier from './NetworkIdentifier.js';

/**
 * The NetworkListResponse model module.
 * @module model/NetworkListResponse
 * @version 1.4.13
 */
class NetworkListResponse {
	/**
	 * Constructs a new <code>NetworkListResponse</code>.
	 * A NetworkListResponse contains all NetworkIdentifiers that the node can serve information for.
	 * @alias module:model/NetworkListResponse
	 * @param networkIdentifiers {Array.<module:model/NetworkIdentifier>}
	 */
	constructor(networkIdentifiers) {

		NetworkListResponse.initialize(this, networkIdentifiers);
	}

	/**
	 * Initializes the fields of this object.
	 * This method is used by the constructors of any subclasses, in order to implement multiple inheritance (mix-ins).
	 * Only for internal use.
	 */
	static initialize(obj, networkIdentifiers) {
		obj['network_identifiers'] = networkIdentifiers;
	}

	/**
	 * Constructs a <code>NetworkListResponse</code> from a plain JavaScript object, optionally creating a new instance.
	 * Copies all relevant properties from <code>data</code> to <code>obj</code> if supplied or a new instance if not.
	 * @param {Object} data The plain JavaScript object bearing properties of interest.
	 * @param {module:model/NetworkListResponse} obj Optional instance to populate.
	 * @return {module:model/NetworkListResponse} The populated <code>NetworkListResponse</code> instance.
	 */
	static constructFromObject(data, obj) {
		if (data) {
			obj = obj || new NetworkListResponse();

			if (data.hasOwnProperty('network_identifiers')) {
				obj['network_identifiers'] = ApiClient.convertToType(data['network_identifiers'], [NetworkIdentifier]);
			}
		}
		return obj;
	}

	/**
	 * Validates the JSON data with respect to <code>NetworkListResponse</code>.
	 * @param {Object} data The plain JavaScript object bearing properties of interest.
	 * @return {boolean} to indicate whether the JSON data is valid with respect to <code>NetworkListResponse</code>.
	 */
	static validateJSON(data) {
		// check to make sure all required properties are present in the JSON string
		for (const property of NetworkListResponse.RequiredProperties) {
			if (!data.hasOwnProperty(property)) {
				throw new Error("The required field `" + property + "` is not found in the JSON data: " + JSON.stringify(data));
			}
		}
		if (data['network_identifiers']) { // data not null
			// ensure the json data is an array
			if (!Array.isArray(data['network_identifiers'])) {
				throw new Error("Expected the field `network_identifiers` to be an array in the JSON data but got " + data['network_identifiers']);
			}
			// validate the optional field `network_identifiers` (array)
			for (const item of data['network_identifiers']) {
				NetworkIdentifier.validateJSON(item);
			};
		}

		return true;
	}


}

NetworkListResponse.RequiredProperties = ["network_identifiers"];

/**
 * @member {Array.<module:model/NetworkIdentifier>} network_identifiers
 */
NetworkListResponse.prototype['network_identifiers'] = undefined;






export default NetworkListResponse;

