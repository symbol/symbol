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
/**
* Enum class Direction.
* @enum {}
* @readonly
*/
export default class Direction {

		/**
		 * value: "forward"
		 * @const
		 */
		"forward" = "forward";


		/**
		 * value: "backward"
		 * @const
		 */
		"backward" = "backward";



	/**
	* Returns a <code>Direction</code> enum value from a Javascript object name.
	* @param {Object} data The plain JavaScript object containing the name of the enum value.
	* @return {module:model/Direction} The enum <code>Direction</code> value.
	*/
	static constructFromObject(object) {
		return object;
	}
}

