/* tslint:disable */
/* eslint-disable */
/**
* @param {number} thermostat_temperature
* @param {number} latitude
* @param {number} longitude
* @param {number} num_occupants
* @param {number} house_size
* @param {string} postcode
* @param {number} epc_space_heating
* @param {number} tes_volume_max
* @returns {string}
*/
export function run_simulation(thermostat_temperature: number, latitude: number, longitude: number, num_occupants: number, house_size: number, postcode: string, epc_space_heating: number, tes_volume_max: number): string;

export type InitInput = RequestInfo | URL | Response | BufferSource | WebAssembly.Module;

export interface InitOutput {
  readonly memory: WebAssembly.Memory;
  readonly run_simulation: (a: number, b: number, c: number, d: number, e: number, f: number, g: number, h: number, i: number, j: number) => void;
  readonly __wbindgen_add_to_stack_pointer: (a: number) => number;
  readonly __wbindgen_malloc: (a: number) => number;
  readonly __wbindgen_realloc: (a: number, b: number, c: number) => number;
  readonly __wbindgen_free: (a: number, b: number) => void;
}

/**
* If `module_or_path` is {RequestInfo} or {URL}, makes a request and
* for everything else, calls `WebAssembly.instantiate` directly.
*
* @param {InitInput | Promise<InitInput>} module_or_path
*
* @returns {Promise<InitOutput>}
*/
export default function init (module_or_path?: InitInput | Promise<InitInput>): Promise<InitOutput>;
