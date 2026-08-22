export interface DisplayType {
	height: number;
	width: number;
	id: string;
	area_top: number;
	area_width: number;
	rails_height: number;
}

export interface DeviceConfig {
	display: DisplayType;
	request_limit: number;
};

export interface DataPaging {
	offset: number;
	count: number;
};

export interface DisplayData {
	dataPaging: DataPaging;
	bitmap: ArrayBuffer;
	forceRefreshSeq: number;
}

export interface Scroll {
	pos: number;
}

export interface KeyPress {
	key: number;
	down: number;
}

export enum WorkMode {
	Stop = 0,
	Run = 1,
	Debug = 2
}

export interface StoredImage {
	height: number;
	width: number;
	dataUrl: string;
}