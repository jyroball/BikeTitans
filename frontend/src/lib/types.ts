// src/lib/types.ts
export interface LocationData {
	total_slot_number: number;
	open_slots: number;
}

export interface LocationsMap {
	[location: string]: LocationData;
}

export interface TableRow {
	location: string;
	total_slot_number: number;
	open_slots: number;
}
